#pragma once

#include "CoreMinimal.h"

#include "FGSubsystem.h"
#include "FGSchematicManager.h"
#include "Buildable/NogsBuildableResearcher.h"
#include "FGResearchManager.h"

#include "FGCharacterPlayer.h"
#include "FGInventoryLibrary.h"

#include "FGInventoryComponent.h"
#include "Patching/BlueprintHookHelper.h"
#include "Patching/BlueprintHookManager.h"
#include "Registry/ModContentRegistry.h"
#include "NogsResearchSubsystem.generated.h"


UCLASS(Abstract, Blueprintable)
class NOGSRESEARCH_API ANogsResearchSubsystem : public AFGSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

		ANogsResearchSubsystem();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Not replicating this , only replicating stored amounts on request or something
	UPROPERTY()
		AFGSchematicManager* SManager;
	UPROPERTY()
		AFGResearchManager* RManager;

	// Begin IFGSaveInterface
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) override {};
	virtual bool NeedTransform_Implementation() override { return false; };
	virtual bool ShouldSave_Implementation() const override { return true; };
	// End IFSaveInterface

	virtual void Tick(float dt) override;

protected:
	TArray<FItemAmount> GetMissingItems(TSubclassOf<class UFGSchematic> Item) const;

	void TickMAMResearch();
	void TickSchematicResearch();

	// FrameIterationIndex ; Optimization solution
	int32 BuildingIterateIndexHUB;
	// FrameIterationIndex ; Optimization solution
	int32 BuildingIterateIndexMAM;

public:

	// This is called when the Buildings lose power, they will unregister themselves which causes the Time needed to Research to change
	UFUNCTION(BlueprintCallable, Category = "Research")
		void ReCalculateSciencePower();

	// Returns the total Schematic wait time in seconds with Science Power Reduction adjusted (either MAM or HUB, this will route it correctly)
	UFUNCTION(BlueprintPure, Category = "Research")
		float GetSchematicDurationAdjusted(TSubclassOf<class UFGSchematic> schematic) const;

	// Returns the remaining Schematic wait time in seconds with Science Power Reduction (factors in elapsed time so far) (either MAM or HUB, this will route it correctly)
	UFUNCTION(BlueprintPure, Category = "Research")
		float GetSchematicProgression(TSubclassOf<class UFGSchematic> schematic) const;

	UFUNCTION(BlueprintPure, Category = "Research", DisplayName = "GetNogsResearchManager", Meta = (DefaultToSelf = "WorldContext"))
		static ANogsResearchSubsystem* Get(class UObject* WorldContext);

protected:
	friend class ANogsBuildableResearcher;
	void RegisterResearcher(ANogsBuildableResearcher* Researcher);
	void UnRegisterResearcher(ANogsBuildableResearcher* Researcher);

	bool GrabItems(UFGInventoryComponent* Inventory, TSubclassOf<class UFGSchematic> Item);

	// Reinit the slot sizes and filters on the mam buffer inventory from the QueueItemMAM
	UFUNCTION(BlueprintCallable)
		void UpdateMAMBufferFilters(bool dumpContents);

public:
	// TODO is this actually used? can't find any usages in bp or cpp
	// This event is called once the Init Process is Done 
	// The BP Version will iterate Schematic Research Trees and process their Dependencies
	UFUNCTION(BlueprintImplementableEvent)
		void PopulateSchematicResearchTreeParents();

	// Cause the MAM buffer inventory to be dropped as pickup items by the HUB terminal
	// Uses base game's FGItemPickup_Spawnable methods to do this
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void DumpMAMBufferToHUBTerminalGround();

	virtual void DumpMAMBufferToHUBTerminalGround_Implementation();

	// Add a Schematic to the Queue (either MAM or HUB, this will route it correctly)
	UFUNCTION(BlueprintCallable)
		bool QueueSchematic(TSubclassOf<class UFGSchematic> Schematic);
	// Remove a Schematic from the Queue (either MAM or HUB, this will route it correctly)
	UFUNCTION(BlueprintCallable)
		bool RemoveQueueSchematic(TSubclassOf<class UFGSchematic> Schematic);

	// Current target item from the HUB Queue, no longer element of the queue
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueueItemHUB;
	// Item we just submitted and are waiting out from the HUB Queue
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueueItemLockedHUB;
	// Queue containing user-ordered HUB items to research
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TArray<TSubclassOf<class UFGSchematic>> QueueHUB;

	// Current target item from the MAM Queue, no longer element of the queue
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueueItemMAM;
	// Item we just submitted and are waiting out from the MAM Queue
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueueItemLockedMAM;
	// Queue containing user-ordered MAM items to research
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TArray<TSubclassOf<class UFGSchematic>> QueueMAM;


	// the Time we spent researching the QueueItemLockedHUB
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		float TimeSpentHUB;
	// the Time we spent researching the QueueItemLockedMAM
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		float TimeSpentMAM;

	// get time reduction percentage value from the current curve asset or 0.f if none
	UFUNCTION(BlueprintPure)
		float GetTimeReductionFactor() const;

	UPROPERTY(savegame, BlueprintReadOnly)
		TArray< ANogsBuildableResearcher* > BuiltResearchers;

	// Holding inventory we pull from for MAM researches (they must all come from one inventory at the time of submit)
	UPROPERTY(SaveGame, BlueprintReadOnly, Replicated)
		class UFGInventoryComponent* mBufferInventoryMAM;

	// Calculated sum of science power from researchers, used to reduce schematic time
	UPROPERTY(BlueprintReadOnly, Replicated)
		int32 TotalSciencePower;

	// Curve to use with Science Points to calculate time reduction
	UPROPERTY(SaveGame, EditDefaultsOnly, BlueprintReadWrite)
		UCurveFloat* ScienceTimeReductionCurve;

	// Item to use as the cost for alternate recipes (since they don't have a cost normally)
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UFGItemDescriptor > AlternateRecipeCostDescriptor;

	// Quantity for AlternateRecipeCostDescriptor
	UPROPERTY(EditDefaultsOnly)
		int32 AlternateRecipeCostQuantity;

	UPROPERTY(BlueprintReadWrite)
		TMap<TSubclassOf<UFGSchematic>, TSubclassOf<UFGResearchTree>> ResearchTreeParents;


private:
	// blanket check if an item can be put into the MAM buffer, the AllowedItemOnIndex and ArbitrarySlotSize set on each slot make it more specific
	UFUNCTION()
		bool VerifyMAMBufferItemTransfer(TSubclassOf<UFGItemDescriptor> ItemClass, int32 Amount) const;
};

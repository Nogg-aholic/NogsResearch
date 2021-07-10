

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



/**
 * 
 */



DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWidgetCreated, UUserWidget*, Widget);


UCLASS(Abstract, Blueprintable)
class NOGSRESEARCH_API ANogsResearchSubsystem : public AFGSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

		ANogsResearchSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Research")
	void BindOnWidgetConstruct(const TSubclassOf<UUserWidget> WidgetClass, FOnWidgetCreated Binding);

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Not replicating this , only replicating stored amounts on request or something

	UPROPERTY()
	AFGSchematicManager * SManager;
	UPROPERTY()
	AFGResearchManager * RManager;
	
	// Begin IFGSaveInterface
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
	virtual void GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) override {};
	virtual bool NeedTransform_Implementation() override { return false; };
	virtual bool ShouldSave_Implementation() const override {return true;};
	// End IFSaveInterface

	virtual void Tick(float dt) override;

protected:
	TArray<FItemAmount> GetMissingItems(TSubclassOf<class UFGSchematic> Item) const;
	
	void TickMAMResearch();
	void TickSchematicResearch();
	
	// FrameIterationIndex ; Optimization solution
	int32 Index;
	int32 IndexSchematic;
public:
	
	// This is called when the Buildings lose power, they will unregister themselfs which causes the Time needed to Research to change
	UFUNCTION(BlueprintCallable, Category = "Research")
	void ReCalculateSciencePower();

	// Returns the Schematic Time with Science Power Reduction adjusted
	UFUNCTION(BlueprintPure, Category = "Research")
		float GetSchematicDurationAdjusted(TSubclassOf<class UFGSchematic> schematic) const;

	// The same as above but already Spent time on this is subtracted
	UFUNCTION(BlueprintPure, Category = "Research")
		float GetSchematicProgression(TSubclassOf<class UFGSchematic> schematic) const;

	UFUNCTION(BlueprintPure, Category = "Research", DisplayName = "GetNogsResearchManager", Meta = (DefaultToSelf = "WorldContext"))
        static ANogsResearchSubsystem* Get(class UObject* WorldContext);

protected:
	friend class ANogsBuildableResearcher;
	void RegisterResearcher(ANogsBuildableResearcher * Researcher);
	void UnRegisterResearcher(ANogsBuildableResearcher * Researcher);
 
 	bool GrabItems(UFGInventoryComponent* Inventory, TSubclassOf<class UFGSchematic> Item);
public:	
	// This event is called once the Init Process is Done 
	// The BP Version will iterate Schematic Research Trees and process their Dependencies
	UFUNCTION(BlueprintImplementableEvent)
		void PopulateSchematicResearchTreeParents();
	
	// Add a Schematic to the Que
	UFUNCTION(BlueprintCallable)
		bool QueSchematic(TSubclassOf<class UFGSchematic> Schematic);
	// Remove a Schematic of the Que
	UFUNCTION(BlueprintCallable)
		bool RemoveQueSchematic(TSubclassOf<class UFGSchematic> Schematic);

	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueItem;
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueItemLocked;
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TArray<TSubclassOf<class UFGSchematic>> Queue;
	
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueItemMAM;
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueItemLockedMAM;
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TArray<TSubclassOf<class UFGSchematic>> QueueMAM;


	// the Time we spent on the QueItemLocked
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		float TimeSpent;
	// Same for MAM
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		float TimeSpentMAM;

	UFUNCTION(BlueprintPure)
		float GetTimeReductionFactor() const;

	UPROPERTY(savegame, BlueprintReadOnly)
		TArray< ANogsBuildableResearcher * > Researcher;

	UPROPERTY(SaveGame,BlueprintReadOnly, Replicated)
		class UFGInventoryComponent* mBufferInventory;
	UPROPERTY(BlueprintReadOnly,Replicated)
		int32 TotalSciencePower;
	UPROPERTY(EditDefaultsOnly)
		UCurveFloat *  ScienceTimeReductionCurve;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UFGItemDescriptor > AlternateRecipeCostDescriptor;
	UPROPERTY(BlueprintReadWrite)
		TMap<TSubclassOf<UFGSchematic>,TSubclassOf<UFGResearchTree>> ResearchTreeParents;



	private:
	UFUNCTION()
    bool VerifyItem(TSubclassOf<UFGItemDescriptor> ItemClass, int32 Amount) const;

	static FOnWidgetCreated OnWidgetCreated;


};

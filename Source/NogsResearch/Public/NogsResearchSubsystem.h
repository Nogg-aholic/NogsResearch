

#pragma once

#include "CoreMinimal.h"
#include "FGSubsystem.h"
#include "FGSchematicManager.h"
#include "Buildable/NogsBuildableResearcher.h"
#include "FGResearchManager.h"

#include "FGCharacterPlayer.h"
#include "FGInventoryLibrary.h"

#include "FGInventoryComponent.h"

#include "NogsResearchWorldSubsystem.h"

#include "Registry/ModContentRegistry.h"
#include "NogsResearchSubsystem.generated.h"



/**
 * 
 */





UCLASS(Blueprintable)
class NOGSRESEARCH_API ANogsResearchSubsystem : public AFGSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

		ANogsResearchSubsystem();
	virtual void BeginPlay() override;



	bool GrabItems(UFGInventoryComponent * Inventory);


	// Not replicating this , only replicating stored amounts on request or something

	UPROPERTY()
	UNogsResearchWorldSubsystem * Subsystem;
	UPROPERTY()
	AFGSchematicManager * SManager;
	UPROPERTY()
	AFGResearchManager * RManager;
	UPROPERTY()
	AModContentRegistry * ContentManager;
	
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

	TArray<FItemAmount> GetMissingItems() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TickMAMResearch();

	void TickSchematicResearch();
	
	// FrameIterationIndex ; Optimization solution
	int32 Index;

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

	// Function for Researchers to Register themselfs 
	// This caues them to be included for Inventory Item searches as well as counting their Science Points towards the Total Science Points Value
	void RegisterResearcher(ANogsBuildableResearcher * Researcher);
	// Called when buildings are deleted or lose Power
	void UnRegisterResearcher(ANogsBuildableResearcher * Researcher);

	// Simple Get Function which doesnt auto create this actor bc that may cause this actor to be created before it should
	// Actor is created after Begin Play of Initmod + .5 Sec 
	// After that the Actor will persist on saves and should be reachable then when its loaded
	UFUNCTION(BlueprintPure, Category = "Research", DisplayName = "GetNogsResearchManager", Meta = (DefaultToSelf = "worldContext"))
		static ANogsResearchSubsystem* Get(class UObject* worldContext);
	
	// This event is called once the Init Process is Done 
	// The BP Version will iterate Schematic Research Trees and process their Dependencies
	UFUNCTION(BlueprintImplementableEvent)
		void PopulateSchematicResearchTreeParents();
	
	UFUNCTION(BlueprintImplementableEvent)
		TArray<TSubclassOf<class UFGSchematic>> GetResearchTreeSchematics();

	UFUNCTION(BlueprintImplementableEvent)
		FNogs_Recipe HandleRecipe();


	// Add a Schematic to the Que
	UFUNCTION(BlueprintCallable)
		bool QueSchematic(TSubclassOf<class UFGSchematic> Schematic);
	// Remove a Schematic of the Que
	UFUNCTION(BlueprintCallable)
		bool RemoveQueSchematic(TSubclassOf<class UFGSchematic> Schematic);

	// The item currently in Focus
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueItem;
	// The item we have a cooldown because of
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TSubclassOf<class UFGSchematic> QueItemLocked;

	// all Schematics currently in Que (Ordered)
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		TArray<TSubclassOf<class UFGSchematic>> Queue;
	// the Time we spent on the QueItemLocked
	UPROPERTY(savegame, BlueprintReadWrite, Replicated)
		float TimeSpent;

	// Return the Percentage 0.f - 1.f  Science Points are Reducing the Time of Schematics by
	UFUNCTION(BlueprintPure)
	float GetTimeReductionFactor() const
	{
		if (!ScienceTimeReductionCurve)
			return 0.f;
		return ScienceTimeReductionCurve->GetFloatValue(TotalSciencePower);
	}

	// All Reasearcher currently Registered ( excludes unpowered ones ) 
	UPROPERTY(savegame, BlueprintReadOnly)
		TArray< ANogsBuildableResearcher * > Researcher;


	// we use this for paying off ResearchTree stuff
	UPROPERTY(SaveGame,BlueprintReadOnly, Replicated)
		class UFGInventoryComponent* mBufferInventory;

	// current Science Power used to Calculate the Time Reduction
	UPROPERTY(BlueprintReadOnly,Replicated)
		int32 TotalSciencePower;

	// Curve we use to Scale TotalSciencePower to a value between 0-1
	UPROPERTY(EditDefaultsOnly)
		UCurveFloat *  ScienceTimeReductionCurve;

	// this is a descriptor we add as cost for all alternate Recipe Schematics ( they dont have a cost by default ) 
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UFGItemDescriptor > AlternateRecipeCostDescriptor;
};

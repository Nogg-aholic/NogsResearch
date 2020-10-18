

#pragma once

#include "CoreMinimal.h"
#include "FGSubsystem.h"
#include "FGSchematicManager.h"
#include "NogsBuildableResearcher.h"
#include "FGResearchManager.h"
#include "FGBuildingDescriptor.h"
#include "FGSchematic.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryLibrary.h"
#include "FGUnlockRecipe.h"
#include "FGUnlockSchematic.h"
#include "FGItemCategory.h"
#include "FGBuildGun.h"
#include "FGSchematicPurchasedDependency.h"
#include "../SML/mod/actor/SMLInitMod.h"
#include "FGInventoryComponent.h"
#include "NogsResearchSubsystem.generated.h"



/**
 * 
 */




USTRUCT(BlueprintType)
struct  NOGSRESEARCH_API  FNogs_Recipe
{
	GENERATED_BODY()
public:
	FNogs_Recipe();
	FNogs_Recipe(TSubclassOf< class UFGRecipe > inClass);
	FNogs_Recipe(TSubclassOf< class UFGRecipe > inClass, TSubclassOf< class UFGSchematic > Schematic);

	UPROPERTY(BlueprintReadWrite)
		TArray<TSubclassOf<class UFGSchematic>> nUnlockedBy;
	UPROPERTY(BlueprintReadWrite)
		TSubclassOf< class UFGRecipe > nRecipeClass;

	TArray<TSubclassOf<class UFGItemDescriptor>> Products()
	{
		TArray<TSubclassOf<class UFGItemDescriptor>> out;
		TArray<FItemAmount> arr = nRecipeClass.GetDefaultObject()->GetProducts();
		for (int32 i = 0; i < arr.Num(); i++)
		{
			out.Add(arr[i].ItemClass);
		}
		return out;
	}

	TArray<TSubclassOf<class UFGItemDescriptor>> Ingredients()
	{
		TArray<TSubclassOf<class UFGItemDescriptor>> out;
		TArray<FItemAmount> arr = nRecipeClass.GetDefaultObject()->GetIngredients();
		for (int32 i = 0; i < arr.Num(); i++)
		{
			out.Add(arr[i].ItemClass);
		}
		return out;
	}

	TArray<TSubclassOf< class UFGItemCategory >> ProductCats()
	{
		TArray<TSubclassOf< class UFGItemCategory >> out;
		TArray<FItemAmount> arr = nRecipeClass.GetDefaultObject()->GetProducts();
		for (int32 i = 0; i < arr.Num(); i++)
		{
			if (!out.Contains(arr[i].ItemClass.GetDefaultObject()->GetItemCategory(arr[i].ItemClass)))
				out.Add(arr[i].ItemClass.GetDefaultObject()->GetItemCategory(arr[i].ItemClass));
		}
		return out;
	}

	TArray<TSubclassOf< class UFGItemCategory >> IngredientCats()
	{
		TArray<TSubclassOf< class UFGItemCategory >> out;
		TArray<FItemAmount> arr = nRecipeClass.GetDefaultObject()->GetIngredients();
		for (int32 i = 0; i < arr.Num(); i++)
		{
			if(!out.Contains(arr[i].ItemClass.GetDefaultObject()->GetItemCategory(arr[i].ItemClass)))
				out.Add(arr[i].ItemClass.GetDefaultObject()->GetItemCategory(arr[i].ItemClass));
		}
		return out;
	}

	~FNogs_Recipe() = default;

};

USTRUCT(BlueprintType)
struct  NOGSRESEARCH_API  FNogs_Schematic
{
	GENERATED_BODY()
public:
	FNogs_Schematic();
	FNogs_Schematic(TSubclassOf< class UFGSchematic > inClass);

	UPROPERTY(BlueprintReadWrite)
		TSubclassOf< class UFGSchematic > nClass;

	UPROPERTY(BlueprintReadWrite)
		TArray<TSubclassOf<class UFGSchematic>> nDependsOn;

	UPROPERTY(BlueprintReadWrite)
		TArray<TSubclassOf<class UFGSchematic>> nDependingOnThis;

	UPROPERTY(BlueprintReadWrite)
		TArray<TSubclassOf<class UFGSchematic>> nVisibilityDepOn;

	UPROPERTY(BlueprintReadWrite)
		TArray<TSubclassOf<class UFGSchematic>> nVisibilityDepOnThis;

	~FNogs_Schematic() = default;
};

UCLASS(Blueprintable)
class NOGSRESEARCH_API ANogsResearchSubsystem : public AFGSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

		ANogsResearchSubsystem();
	virtual void BeginPlay() override;




	void HandleSchematic(TSubclassOf<class UFGSchematic> Schematic);



	bool GrabItems(UFGInventoryComponent * Inventory);


	// Not replicating this , only replicating stored amounts on request or something


	AFGSchematicManager * SManager;
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

	TArray<FItemAmount> GetMissingItems();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TickMAMResearch();

	void TickSchematicResearch();

public:

	// Executed from Init mod on Server and on Client 
	// while most parts are server side replicated Schematic and Recipe Array are Client side Generated
	UFUNCTION(BlueprintCallable, Category = "Research")
	void Init();

	// This is called when the Buildings lose power, they will unregister themselfs which causes the Time needed to Research to change
	UFUNCTION(BlueprintCallable, Category = "Research")
	void ReCalculateSciencePower();

	// Returns the Schematic Time with Science Power Reduction adjusted
	UFUNCTION(BlueprintPure, Category = "Research")
		float GetSchematicDurationAdjusted(TSubclassOf<class UFGSchematic> schematic);
	// The same as above but already Spent time on this is subtracted
	UFUNCTION(BlueprintPure, Category = "Research")
		float GetSchematicProgression(TSubclassOf<class UFGSchematic> schematic);

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
	float GetTimeReductionFactor() {
		if (!ScienceTimeReductionCurve)
			return 0.f;
		return ScienceTimeReductionCurve->GetFloatValue(TotalSciencePower);
	}

	// All Reasearcher currently Registered ( excludes unpowered ones ) 
	UPROPERTY(savegame, BlueprintReadOnly)
		TArray< ANogsBuildableResearcher * > Researcher;

	// all research Trees 
	UPROPERTY(Transient, BlueprintReadOnly)
		TArray<TSubclassOf<class UFGResearchTree>> nResearchTrees;

	// all Schematics
	UPROPERTY(Transient, BlueprintReadOnly)
		TArray< TSubclassOf< class UFGSchematic > > mSchematics;
	
	// schematics in a struct with easy dependency access
	UPROPERTY(BlueprintReadWrite, Category = "Info")
		TMap<TSubclassOf<class UFGSchematic>, FNogs_Schematic> nSchematics;

	// all Recipes
	UPROPERTY(BlueprintReadOnly, Category = "Info")
		TArray<TSubclassOf<class UFGRecipe>> mRecipes;
	// all Recipes in a struct with easy access to some Properties
	UPROPERTY(BlueprintReadWrite, Category = "Info")
		TMap<TSubclassOf<class UFGRecipe>, FNogs_Recipe > nRecipes;

	// this suxxs.. should add the research tree to the schematic struct maybe ? 
	// its possible to have a schematic in 2 Trees, which this map would ignore
	UPROPERTY(Transient, BlueprintReadWrite)
		TMap< TSubclassOf<class UFGSchematic>, TSubclassOf<class UFGResearchTree>> SchematicResearchTreeParents;

	// easy access to descriptors -> buildable class and reverse for finding Building Icons 
	UPROPERTY(BlueprintReadOnly, Category = "Info")
		TMap<TSubclassOf< UFGBuildingDescriptor >, TSubclassOf< class AFGBuildable >> nBuildGunBuildings;

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

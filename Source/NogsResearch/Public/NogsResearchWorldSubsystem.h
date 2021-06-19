#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Resources/FGBuildingDescriptor.h"
#include "Unlocks/FGUnlockRecipe.h"
#include "Unlocks/FGUnlockSchematic.h"
#include "FGItemCategory.h"
#include "FGSchematic.h"
#include "FGRecipe.h"
#include "Equipment/FGBuildGun.h"
#include "AvailabilityDependencies/FGSchematicPurchasedDependency.h"
#include "NogsResearchWorldSubsystem.generated.h"



USTRUCT(BlueprintType)
struct  NOGSRESEARCH_API  FNogs_Recipe
{
	GENERATED_BODY()
public:
	FNogs_Recipe();
	FNogs_Recipe(TSubclassOf< class UFGRecipe > InClass);
	FNogs_Recipe(TSubclassOf< class UFGRecipe > InClass, TSubclassOf< class UFGSchematic > Schematic);

	UPROPERTY(BlueprintReadWrite)
		TArray<TSubclassOf<class UFGSchematic>> nUnlockedBy;
	UPROPERTY(BlueprintReadWrite)
		TSubclassOf< class UFGRecipe > nRecipeClass;

	TArray<TSubclassOf<class UFGItemDescriptor>> Products() const;

	TArray<TSubclassOf<class UFGItemDescriptor>> Ingredients() const;

	TArray<TSubclassOf<class UFGItemCategory>> ProductCats() const;

	TArray<TSubclassOf<class UFGItemCategory>> IngredientCats() const;

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


USTRUCT(BlueprintType)
struct  NOGSRESEARCH_API  FNogs_ResearchSchematic
{
	GENERATED_BODY()
public:
	FNogs_ResearchSchematic() {
		Schematic = FNogs_Schematic();
		Coordinates = FVector2D();
		Parents = {};
		Children = {};
	};
	
	FNogs_Schematic Schematic;
	
	UPROPERTY(BlueprintReadWrite)
	FVector2D Coordinates;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FVector2D> Parents;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FVector2D> Children;
};


class ANogsResearchSubsystem;

/**
 * 
 */
UCLASS()
class NOGSRESEARCH_API UNogsResearchWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

		/** Implement this for initialization of instances of the system */
		virtual void Initialize(FSubsystemCollectionBase& Collection) override;

		/** Implement this for deinitialization of instances of the system */
		virtual void Deinitialize()override;
public:
		UFUNCTION(BlueprintCallable)
			FNogs_Schematic HandleSchematic(TSubclassOf<class UFGSchematic> Schematic);

		UFUNCTION(BlueprintCallable)
			void HandleSchematicFromTree(TSubclassOf<class UFGSchematic> Schematic, TSubclassOf<class UFGResearchTree> Tree, FVector2D Coords, TArray<FVector2D> Parents, TArray<FVector2D> Children);



		UFUNCTION(BlueprintCallable)
		void ClientInit();

		// all research Trees 
		UPROPERTY(Transient, BlueprintReadOnly)
			TArray<TSubclassOf<class UFGResearchTree>> nResearchTrees;
		// schematics in a struct with easy dependency access
		UPROPERTY(BlueprintReadWrite, Category = "Info")
			TMap<TSubclassOf<class UFGSchematic>, FNogs_Schematic> nSchematics;
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

		UPROPERTY()
		ANogsResearchSubsystem * nResearchSubsystem;
};

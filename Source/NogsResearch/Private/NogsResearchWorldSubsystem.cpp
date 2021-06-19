#include "NogsResearchWorldSubsystem.h"
#include "NogsResearchSubsystem.h"

// Struct constructors
FNogs_Schematic::FNogs_Schematic() {
	nClass = nullptr;
	nDependsOn = {};
	nDependingOnThis = {};
	nVisibilityDepOn = {};
	nVisibilityDepOnThis = {};
};
FNogs_Schematic::FNogs_Schematic(TSubclassOf<UFGSchematic> inClass) { 
	nClass = inClass;
	nDependsOn = {};
	nDependingOnThis = {};
	nVisibilityDepOn = {};
	nVisibilityDepOnThis = {};
};

FNogs_Recipe::FNogs_Recipe() {
	nRecipeClass = nullptr;
	nUnlockedBy = {};
};
FNogs_Recipe::FNogs_Recipe(TSubclassOf<UFGRecipe> InClass) { nRecipeClass = InClass;
	nUnlockedBy = {};
}
FNogs_Recipe::FNogs_Recipe(TSubclassOf<UFGRecipe> InClass, TSubclassOf<UFGSchematic> Schematic) { 
	nRecipeClass = InClass;	
	nUnlockedBy.Add(Schematic); 
}

TArray<TSubclassOf<UFGItemDescriptor>> FNogs_Recipe::Products() const
{
	TArray<TSubclassOf<class UFGItemDescriptor>> out;
	TArray<FItemAmount> Arr = nRecipeClass.GetDefaultObject()->GetProducts();
	for (auto i : Arr)
	{
		out.Add(i.ItemClass);
	}
	return out;
}

TArray<TSubclassOf<UFGItemDescriptor>> FNogs_Recipe::Ingredients() const
{
	TArray<TSubclassOf<class UFGItemDescriptor>> out;
	TArray<FItemAmount> Arr = nRecipeClass.GetDefaultObject()->GetIngredients();
	for (auto i : Arr)
	{
		out.Add(i.ItemClass);
	}
	return out;
}

TArray<TSubclassOf<UFGItemCategory>> FNogs_Recipe::ProductCats() const
{
	TArray<TSubclassOf<class UFGItemCategory>> Out;
	TArray<FItemAmount> Arr = nRecipeClass.GetDefaultObject()->GetProducts();
	for (auto i : Arr)
	{
		if (!Out.Contains(i.ItemClass.GetDefaultObject()->GetItemCategory(i.ItemClass)))
			Out.Add(i.ItemClass.GetDefaultObject()->GetItemCategory(i.ItemClass));
	}
	return Out;
}

TArray<TSubclassOf<UFGItemCategory>> FNogs_Recipe::IngredientCats() const
{
	TArray<TSubclassOf<class UFGItemCategory>> Out;
	TArray<FItemAmount> Arr = nRecipeClass.GetDefaultObject()->GetIngredients();
	for (auto i : Arr)
	{
		if (!Out.Contains(i.ItemClass.GetDefaultObject()->GetItemCategory(i.ItemClass)))
			Out.Add(i.ItemClass.GetDefaultObject()->GetItemCategory(i.ItemClass));
	}
	return Out;
}


void UNogsResearchWorldSubsystem::Initialize(FSubsystemCollectionBase & Collection)
{
	Super::Initialize(Collection);
}

void UNogsResearchWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}


FNogs_Schematic UNogsResearchWorldSubsystem::HandleSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	FNogs_Schematic NewEntry = FNogs_Schematic(Schematic);

	// Iterate deps and add them to Schematic Struct
	TArray<  UFGAvailabilityDependency* > Out_SchematicDependencies;
	Schematic.GetDefaultObject()->GetSchematicDependencies(Schematic, Out_SchematicDependencies);
	for (auto i :  Out_SchematicDependencies)
	{
		if (!i)
			continue;
		TArray< TSubclassOf< class UFGSchematic > > Out_Schematics;
		const UFGSchematicPurchasedDependency* Dep = Cast<UFGSchematicPurchasedDependency>(i);
		if (Dep)
		{
			Dep->GetSchematics(Out_Schematics);
			for (auto j : Out_Schematics)
			{
				if (!j)
					continue;
				if (!NewEntry.nDependsOn.Contains(j))
					NewEntry.nDependsOn.Add(j);
			}
		}
	}
	// Insert Schematic Struct to Map
	nSchematics.Add(Schematic, NewEntry);

	TArray< UFGUnlock* > Unlocks = Schematic.GetDefaultObject()->GetUnlocks(Schematic);
	for (auto i : Unlocks)
	{
		// Recipe unlocks make struct for it and save Buildings found
		if (Cast<UFGUnlockRecipe>(i))
		{
			TArray< TSubclassOf< class UFGRecipe > >  UnlockRecipes = Cast<UFGUnlockRecipe>(i)->GetRecipesToUnlock();
			for (auto j : UnlockRecipes)
			{
				if (!j)
					continue;
				FNogs_Recipe rep;
				if (!nRecipes.Contains(j))
				{
					rep = FNogs_Recipe(j, Schematic);
					nRecipes.Add(j, rep);
				}
				else
				{
					rep = *nRecipes.Find(j);
					rep.nUnlockedBy.Add(Schematic);
				}
				TArray< TSubclassOf< UObject > > BuildClasses;
				j.GetDefaultObject()->GetProducedIn(BuildClasses);
				BuildClasses.Remove(nullptr);
				if (rep.Products().IsValidIndex(0))
				{
					for (const auto l : BuildClasses)
					{
						AFGBuildGun* BuildGun = Cast<AFGBuildGun>(l->GetClass());
						if (BuildGun)
						{
							if (!nBuildGunBuildings.Contains(*rep.Products()[0]))
								nBuildGunBuildings.Add(*rep.Products()[0]);
						}
					}
				}
			}
		} // schematics unlocks cause recursion
		else if (Cast<UFGUnlockSchematic>(i))
		{
			TArray< TSubclassOf< class UFGSchematic > > UnlockSchematics = Cast<UFGUnlockSchematic>(i)->GetSchematicsToUnlock();
			for (auto j : UnlockSchematics)
			{
				if (!nSchematics.Contains(j))
				{
					HandleSchematic(j);
				}
			}
		}
	}

	return NewEntry;
}

void UNogsResearchWorldSubsystem::HandleSchematicFromTree(TSubclassOf<class UFGSchematic> Schematic, TSubclassOf<class UFGResearchTree> Tree, FVector2D Coords, TArray<FVector2D> Parents, TArray<FVector2D> Children)
{
}

void UNogsResearchWorldSubsystem::ClientInit()
{
	UWorld * World = GetWorld();
	nResearchSubsystem  = ANogsResearchSubsystem::Get(World);

	if (!nResearchSubsystem) 
	{
		UE_LOG(LogTemp, Fatal, TEXT("No ResearchSubsystem Found. this was Called too Early !"));
		return;
	}
	else
	{
		TArray< TSubclassOf< class UFGSchematic > > toProcess;
		TArray<FSchematicRegistrationInfo> Schematics;
		TArray<FResearchTreeRegistrationInfo> Research;
		AModContentRegistry * ContentManager = AModContentRegistry::Get(GetWorld());

		if (ContentManager)
		{
			Schematics = ContentManager->GetRegisteredSchematics();
			Research = ContentManager->GetRegisteredResearchTrees();
		}
		else
		{
			UE_LOG(LogTemp, Fatal, TEXT("No AModContentRegistry Found. this was Called too Early !"));
			return;
		}

		for (auto i : Schematics)
		{
			HandleSchematic(i.RegisteredObject);
			toProcess.Add(i.RegisteredObject);
		}
		for (auto  i : Research)
		{
			nResearchTrees.Add(i.RegisteredObject);
		}

		nResearchSubsystem->PopulateSchematicResearchTreeParents();

		toProcess.Remove(nullptr);

		for (auto i : toProcess)
		{
			for (auto k : toProcess)
			{
				TArray<  UFGAvailabilityDependency* > Out_SchematicDependencies;
				k.GetDefaultObject()->GetSchematicDependencies(k, Out_SchematicDependencies);
				for (auto j :  Out_SchematicDependencies)
				{
					if (!j)
						continue;
					if (Cast<UFGSchematicPurchasedDependency>(j))
					{
						TArray< TSubclassOf< class UFGSchematic > > Out_Schematics;
						Cast<UFGSchematicPurchasedDependency>(j)->GetSchematics(Out_Schematics);
						if (Out_Schematics.Contains(i))
						{
							if (!nSchematics.Find(i)->nDependingOnThis.Contains(k))
							{
								nSchematics.Find(i)->nDependingOnThis.Add(k);
							}
						}

					}
				}
			}	
		}
	}
}

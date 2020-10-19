

#include "NogsResearchSubsystem.h"

#include "FactoryGame.h"

// Struct constructors
FNogs_Schematic::FNogs_Schematic(){};
FNogs_Schematic::FNogs_Schematic(TSubclassOf<UFGSchematic> inClass){nClass = inClass;};

FNogs_Recipe::FNogs_Recipe(){};
FNogs_Recipe::FNogs_Recipe(TSubclassOf<UFGRecipe> inClass){	nRecipeClass = inClass;}
FNogs_Recipe::FNogs_Recipe(TSubclassOf<UFGRecipe> inClass, TSubclassOf<UFGSchematic> Schematic){nRecipeClass = inClass;	nUnlockedBy.Add(Schematic);}

// default stuff
ANogsResearchSubsystem::ANogsResearchSubsystem() : Super() {
	this->PrimaryActorTick.TickGroup = TG_PrePhysics; 
	this->PrimaryActorTick.EndTickGroup = TG_PrePhysics; 
	this->PrimaryActorTick.bTickEvenWhenPaused = false; 
	this->PrimaryActorTick.bCanEverTick = true; 
	this->PrimaryActorTick.bStartWithTickEnabled = true;
	this->PrimaryActorTick.bAllowTickOnDedicatedServer = true; 
	this->PrimaryActorTick.TickInterval = 0;
	this->bAlwaysRelevant = true;
	this->bReplicates = true;
}

void ANogsResearchSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANogsResearchSubsystem, TotalSciencePower);
	DOREPLIFETIME(ANogsResearchSubsystem, mBufferInventory);
	DOREPLIFETIME(ANogsResearchSubsystem, QueItem);
	DOREPLIFETIME(ANogsResearchSubsystem, Queue);
	DOREPLIFETIME(ANogsResearchSubsystem, QueItemLocked);
	DOREPLIFETIME(ANogsResearchSubsystem, TimeSpent);

	
}

void ANogsResearchSubsystem::BeginPlay()
{
	Super::BeginPlay();

	// if we dont exclude this for Editor we crash :(

#if WITH_EDITOR
#else
	if (HasAuthority())
	{
		if (!mBufferInventory)
		{
			mBufferInventory = UFGInventoryLibrary::CreateInventoryComponent(this, TEXT("BufferInventory"));
			if (mBufferInventory->GetSizeLinear() != 18)
			{
				mBufferInventory->Resize(18);
			}
		}
		else
		{
			if (mBufferInventory->GetSizeLinear() != 18)
			{
				mBufferInventory->Resize(18);
			}
		}
		for (int32 i = 0; i < mBufferInventory->mArbitrarySlotSizes.Num(); i++)
		{
			mBufferInventory->mArbitrarySlotSizes[i] = 5000;
		}
	}
#endif
	
	SManager = AFGSchematicManager::Get(GetWorld());
	RManager = AFGResearchManager::Get(GetWorld());
}

void ANogsResearchSubsystem::Init()
{
	// happens on both client and remote

	TArray< TSubclassOf< class UFGSchematic > > toProcess;

	if (SManager)
	{
		for (int32 i = 0; i < SManager->mAvailableSchematics.Num(); i++)
		{
			if (!SManager->mAvailableSchematics[i])
				continue;
			if (!toProcess.Contains(SManager->mAvailableSchematics[i]))
			{
				if (!SManager->mAllSchematics.Contains(SManager->mAvailableSchematics[i]))
				{
					SManager->mAllSchematics.Add(SManager->mAvailableSchematics[i]);
				}
				toProcess.Add(SManager->mAvailableSchematics[i]);
			}
		}
		SManager->mAvailableSchematics = toProcess;
	}

	if (SManager)
		SManager->GetAllSchematics(toProcess);

	if (RManager)
		nResearchTrees = RManager->mAllResearchTrees;

	TArray<AActor*> SMLInitActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASMLInitMod::StaticClass(), SMLInitActors);

	for (int32 i = 0; i < SMLInitActors.Num(); i++)
	{
		for (int32 j = 0; j < Cast<ASMLInitMod>(SMLInitActors[i])->mResearchTrees.Num(); j++)
		{
			class TSubclassOf<UFGResearchTree> tree = Cast<ASMLInitMod>(SMLInitActors[i])->mResearchTrees[j];
			if (!nResearchTrees.Contains(tree))
			{
				nResearchTrees.Add(tree);
			}
		}
		for (int32 j = 0; j < Cast<ASMLInitMod>(SMLInitActors[i])->mSchematics.Num(); j++)
		{
			TSubclassOf<UFGSchematic> schem = Cast<ASMLInitMod>(SMLInitActors[i])->mSchematics[j];
			if (!toProcess.Contains(schem))
			{
				toProcess.Add(schem);
			}
		}
		
	}

	for (int32 i = 0; i < toProcess.Num(); i++)
	{
		if (toProcess[i])
		{
			if (!mSchematics.Contains(toProcess[i]))
			{
				HandleSchematic(toProcess[i]);
			}
		}	
	}

	for (int32 i = 0; i < toProcess.Num(); i++)
	{
		if (toProcess[i])
		{
			for (int32 k = 0; k < toProcess.Num(); k++)
			{
				TArray<  UFGAvailabilityDependency* > out_schematicDependencies;
				toProcess[k].GetDefaultObject()->GetSchematicDependencies(toProcess[k], out_schematicDependencies);
				for (int32 j = 0; j < out_schematicDependencies.Num(); j++)
				{
					if (!out_schematicDependencies[j])
						continue;
					TArray< TSubclassOf< class UFGSchematic > > out_schematics;
					if (Cast<UFGSchematicPurchasedDependency>(out_schematicDependencies[j]))
					{
						Cast<UFGSchematicPurchasedDependency>(out_schematicDependencies[j])->GetSchematics(out_schematics);

						if (out_schematics.Contains(toProcess[i]))
						{
							if (!nSchematics.Find(toProcess[i])->nDependingOnThis.Contains(toProcess[k]))
							{
								nSchematics.Find(toProcess[i])->nDependingOnThis.Add(toProcess[k]);
							}
						}

					}
				}
			}
		}
	}

	PopulateSchematicResearchTreeParents();
}


void ANogsResearchSubsystem::ReCalculateSciencePower()
{
	float Power = 0.f;
	for (int32 i = 0; i < Researcher.Num(); i++)
	{
		Power += Researcher[i]->SciencePower;
	}
	TotalSciencePower = Power;
}

void ANogsResearchSubsystem::HandleSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	mSchematics.Add(Schematic);
	FNogs_Schematic newEntry = FNogs_Schematic(Schematic);
	
	// Iterate deps and add them to Schematic Struct
	TArray<  UFGAvailabilityDependency* > out_schematicDependencies;
	Schematic.GetDefaultObject()->GetSchematicDependencies(Schematic, out_schematicDependencies);
	for (int32 i = 0; i < out_schematicDependencies.Num(); i++)
	{
		if (!out_schematicDependencies[i])
			continue;
		TArray< TSubclassOf< class UFGSchematic > > out_schematics;
		if (Cast<UFGSchematicPurchasedDependency>(out_schematicDependencies[i]))
		{
			Cast<UFGSchematicPurchasedDependency>(out_schematicDependencies[i])->GetSchematics(out_schematics);
			for (int32 j = 0; j < out_schematics.Num(); j++)
			{
				if (!out_schematics[j])
					continue;
				if(!newEntry.nDependsOn.Contains(out_schematics[j]))
					newEntry.nDependsOn.Add(out_schematics[j]);

			}
			
		}
	}
	// Insert Schematic Struct to Map
	nSchematics.Add(Schematic,newEntry);  

	TArray< UFGUnlock* > unlocks = Schematic.GetDefaultObject()->GetUnlocks(Schematic);
	for (int32 k = 0; k < unlocks.Num(); k++)
	{
		// Recipe unlocks make struct for it and save Buildings found
		if (Cast<UFGUnlockRecipe>(unlocks[k]))
		{
			TArray< TSubclassOf< class UFGRecipe > >  unlockrecipes = Cast<UFGUnlockRecipe>(unlocks[k])->GetRecipesToUnlock();
			for (int32 j = 0; j < unlockrecipes.Num(); j++)
			{
				if (!unlockrecipes[j])
					continue;
				FNogs_Recipe rep;
				if (!nRecipes.Contains(unlockrecipes[j]))
				{
					mRecipes.Add(unlockrecipes[j]);
					rep = FNogs_Recipe(unlockrecipes[j], Schematic);
					nRecipes.Add(unlockrecipes[j], rep);
				}
				else
				{
					rep = *nRecipes.Find(unlockrecipes[j]);
					rep.nUnlockedBy.Add(Schematic);
				}
				TArray< TSubclassOf< UObject > > buildclasses;
				unlockrecipes[j].GetDefaultObject()->GetProducedIn(buildclasses);
				buildclasses.Remove(nullptr);
				for (int32 i = 0; i < buildclasses.Num(); i++)
				{
					if (rep.Products().IsValidIndex(0))
					{
						AFGBuildGun* buildgun = Cast<AFGBuildGun>(buildclasses[i]->GetClass());
						if (buildgun)
						{
							if (!nBuildGunBuildings.Contains(*rep.Products()[0]))
								nBuildGunBuildings.Add(*rep.Products()[0]);
						}
					}
				}
			}
		} // schematics unlocks cause recursion
		else if (Cast<UFGUnlockSchematic>(unlocks[k]))
		{
			TArray< TSubclassOf< class UFGSchematic > > unlockschematics = Cast<UFGUnlockSchematic>(unlocks[k])->GetSchematicsToUnlock();
			for (int32 j = 0; j < unlockschematics.Num(); j++)
			{
				if (!mSchematics.Contains(unlockschematics[j]))
				{
					HandleSchematic(unlockschematics[j]);
				}
			}
		}
	}
}

void ANogsResearchSubsystem::RegisterResearcher(ANogsBuildableResearcher * building)
{
	if (!Researcher.Contains(building))
	{
		Researcher.Add(building);
	}
	ReCalculateSciencePower();
}

void ANogsResearchSubsystem::UnRegisterResearcher(ANogsBuildableResearcher * building)
{
	if (Researcher.Contains(building))
	{
		Researcher.Remove(building);
	}
	ReCalculateSciencePower();
}

ANogsResearchSubsystem * ANogsResearchSubsystem::Get(UObject * worldContext)
{
	if (!worldContext)
		return nullptr;
	if (worldContext->GetWorld())
	{
		
		TArray<AActor *> arr;
		UGameplayStatics::GetAllActorsOfClass(worldContext->GetWorld(), ANogsResearchSubsystem::StaticClass(), arr);
		if (arr.IsValidIndex(0))
		{
			ANogsResearchSubsystem* out = Cast<ANogsResearchSubsystem>(arr[0]);
			return out;
		}
		else
		{
			return nullptr;
		}
	}
	else
		return nullptr;
}

bool ANogsResearchSubsystem::GrabItems(UFGInventoryComponent * Inventory)
{
	TArray<FItemAmount> cost = GetMissingItems();
	if (cost.Num() == 0)
		return true;

	for (int32 i = 0; i < cost.Num(); i++)
	{
		TArray<TSubclassOf<class UFGItemDescriptor>> lookingFor;
		lookingFor.Add(cost[i].ItemClass);

		TArray<int32> RelevantIndex = Inventory->GetRelevantStackIndexes(lookingFor, 1);

		if (!RelevantIndex.IsValidIndex(0))
			continue;

		FInventoryStack stack;
		Inventory->GetStackFromIndex(RelevantIndex[0], stack);
		if (stack.Item.ItemClass)
		{
			if (stack.Item.ItemClass == cost[i].ItemClass && stack.NumItems > 0)
			{
				int32 remove = FMath::Clamp(stack.NumItems, 0, cost[i].Amount);

				Inventory->RemoveFromIndex(RelevantIndex[0], remove);
				if (QueItem.GetDefaultObject()->mType == ESchematicType::EST_MAM || QueItem.GetDefaultObject()->mType == ESchematicType::EST_Alternate)
				{
					// only if the inventory is not the same we add it, this will be the case when this is an Alternate Schematic and we want to use the remove as payment of the Cost ( no implementation on CSS side for it) 
					if (mBufferInventory != Inventory)
					{
						// if this for a MAM recipe , the input inventory is not mBufferInventory and we simply add it to the Buffer inventory for payment ( CSS Implementation uses inventory to check if we have enough stuff ) 
						stack.NumItems = remove;
						if (mBufferInventory->HasEnoughSpaceForStack(stack))
							mBufferInventory->AddStack(stack);
					}
				}
				else
				{
					// Schematic cost are payed off directly 
					// we dont need to recheck again for what cost is left since this loop is on a descriptor basis and there are no 0 amounts 
					TArray<FItemAmount> payoffArray;
					FItemAmount item = FItemAmount(stack.Item.ItemClass, remove);
					payoffArray.Add(item);
					SManager->PayOffOnSchematic(QueItem, payoffArray);
					if (SManager->IsSchematicPaidOff(QueItem))
					{
						return true;
					}
				}
			}
		}
	}
	return false;

}


void ANogsResearchSubsystem::Tick(float dt)
{
	if (!HasAuthority())
		return;
	if(Researcher.Contains(nullptr))
	{
		// researchers deleting themselfs are cleared here
		Researcher.Remove(nullptr);
	}

	if (!QueItemLocked)
	{
		if (!QueItem)
		{
			if (Queue.IsValidIndex(0))
			{
				if (Queue[0])
				{
					QueItem = Queue[0];
					Queue.RemoveAt(0);
				}
			}
			return;
		}
		switch (QueItem.GetDefaultObject()->mType)
		{
			case ESchematicType::EST_Custom:
			{
				TickSchematicResearch();
				break;
			}
			case ESchematicType::EST_Cheat:
			{
				TickSchematicResearch();
				break;
			}
			case ESchematicType::EST_Tutorial:
			{
				TickSchematicResearch();
				break;
			}
			case ESchematicType::EST_Milestone:
			{
				TickSchematicResearch();
				break;
			}
			case ESchematicType::EST_Alternate:
			{
				TickSchematicResearch();

				break;
			}
			case ESchematicType::EST_Story:
			{
				TickSchematicResearch();
				break;
			}
			case ESchematicType::EST_MAM:
			{
				TickMAMResearch();
				break;
				// TODO parralel research

			}
			case ESchematicType::EST_ResourceSink:
			{
				// even worth it?
				break;
			}
			case ESchematicType::EST_HardDrive:
			{
				break;
				// not used 
			}
		}
	}
	else if (GetSchematicProgression(QueItemLocked) <= 0)
	{
		if (QueItemLocked.GetDefaultObject()->mType == ESchematicType::EST_MAM)
		{
			// bit annoying to have to call this manually without adjust the time itself but works for now to simply call its end Event
			RManager->OnResearchTimerCompleteAccessor(QueItemLocked);
		}
		QueItemLocked = nullptr;
		TimeSpent = 0.f;
	}
	else
	{
		TimeSpent += dt;
	}
}


TArray<FItemAmount> ANogsResearchSubsystem::GetMissingItems()
{
	TArray<FItemAmount> cost;
	if (QueItem.GetDefaultObject()->mType == ESchematicType::EST_MAM)
	{
		cost = QueItem.GetDefaultObject()->mCost;
	}
	else if (QueItem.GetDefaultObject()->mType == ESchematicType::EST_Alternate)
	{
		// cost is usually empty here ..
		// we are adding 2 Hard Drives as additional cost
		// this gets quite complicated later on since we use this function to get only what we dont have already
		cost = QueItem.GetDefaultObject()->mCost; 
		cost.Add(FItemAmount(AlternateRecipeCostDescriptor, 2));
	}
	else
	{
		return cost = SManager->GetRemainingCostFor(QueItem);
	}
	// check the buffer inventory for items we already have and can subtract from what we need
	for (int32 j = 0; j < cost.Num(); j++)
	{
		if (cost[j].Amount == 0)
			continue;

		TArray<TSubclassOf<class UFGItemDescriptor>> lookingFor;
		lookingFor.Add(cost[j].ItemClass);
		TArray<int32> RelevantIndex = mBufferInventory->GetRelevantStackIndexes(lookingFor, 1);
		
		if (!RelevantIndex.IsValidIndex(0))
			continue;

		FInventoryStack stack;
		mBufferInventory->GetStackFromIndex(RelevantIndex[0], stack);
		if (stack.Item.ItemClass)
		{
			if (stack.Item.ItemClass == cost[j].ItemClass && stack.NumItems > 0)
			{
				int32 localamount = cost[j].Amount - stack.NumItems;
				cost[j].Amount = FMath::Clamp(localamount, 0, 100000);
			}
		}
	}
	TArray<FItemAmount> costout;
	for (int32 j = 0; j < cost.Num(); j++)
	{
		if (cost[j].Amount > 0)
		{
			costout.Add(cost[j]);
		}
	}

	return costout;
}


void ANogsResearchSubsystem::TickMAMResearch()
{
	if (RManager->IsResearchComplete(QueItem))
	{
		TArray< TSubclassOf< UFGSchematic > > arr;
		int32 ind = 0;
		AFGCharacterPlayer * character = Cast<AFGCharacterPlayer>(Instigator);
		while (RManager->ClaimResearchResults(character, QueItem, ind))
		{
			ind++;
		}
		QueItem = nullptr;
		return;
	}

	if (RManager->CanResearchBeInitiated(QueItem))
	{
		TArray<FItemAmount> needed = GetMissingItems();
		if (needed.Num() == 0)
		{
			if (SchematicResearchTreeParents.Contains(QueItem))
			{
				RManager->InitiateResearch(mBufferInventory, QueItem, *SchematicResearchTreeParents.Find(QueItem));
				QueItemLocked = QueItem;
			}
			return;
		}

		for (int32 i = 0; i < Researcher.Num(); i++)
		{
			if (Researcher[i]->IsPendingKill())
			{
				Researcher.Remove(Researcher[i]);
				break;
			}

			if (Researcher[i]->HasPower())
			{
				if (GrabItems(Researcher[i]->GetStorageInventory()))
				{
					if (!SchematicResearchTreeParents.Contains(QueItem))
					{
						// TODO add logging :Y
						// should never happen tho :I
						break;
					}

					RManager->InitiateResearch(mBufferInventory, QueItem, *SchematicResearchTreeParents.Find(QueItem));
					QueItemLocked = QueItem;
					ReCalculateSciencePower();
					Queue.Remove(QueItem);
					QueItem = nullptr;
					break;
				}
			}
		}
	}
}


void ANogsResearchSubsystem::TickSchematicResearch()
{
	if (!QueItem)
		return;
	if (SManager->GetActiveSchematic() != QueItem)
	{
		SManager->SetActiveSchematic(QueItem);
	}
		
	for (int32 i = 0; i < Researcher.Num(); i++)
	{
		if (Researcher[i]->IsPendingKill())
		{
			// dont wanna crash here
			Researcher.Remove(Researcher[i]);
			break;
		}
		if (Researcher[i]->HasPower())
		{
			if (!QueItem)
				return;

			if (SManager->IsSchematicPaidOff(QueItem) && QueItem.GetDefaultObject()->mType != ESchematicType::EST_Alternate)
			{
				break;
			}
			
			if (GrabItems(Researcher[i]->GetStorageInventory()))
			{
				break;
			}
		}
		else
		{
		}
	}

	// we check once more on the buffer inventory , if we are already done nothing will happen and we return here
	if (QueItem.GetDefaultObject()->mType == ESchematicType::EST_Alternate)
	{
		TArray<FItemAmount> cost = GetMissingItems();
		if (cost.Num() == 0)
		{
			cost = QueItem.GetDefaultObject()->mCost;
			cost.Add(FItemAmount(AlternateRecipeCostDescriptor, 2));

			for (int32 j = 0; j < cost.Num(); j++)
			{
				if (cost[j].Amount == 0)
					continue;

				mBufferInventory->Remove(cost[j].ItemClass, cost[j].Amount);
				
			}

			SManager->GiveAccessToSchematic(QueItem, false);
			QueItemLocked = QueItem;
			ReCalculateSciencePower();
			Queue.Remove(QueItem);
			QueItem = nullptr;
			return;
		}
	}
	else if(SManager->IsSchematicPaidOff(QueItem))
	{
		SManager->LaunchShip();
		QueItemLocked = QueItem;
		ReCalculateSciencePower();
		Queue.Remove(QueItem);
		QueItem = nullptr;
	}

	

	
}

float ANogsResearchSubsystem::GetSchematicDurationAdjusted(TSubclassOf<class UFGSchematic> schematic)
{
	if (!schematic)
		return 600.f;

	if (schematic.GetDefaultObject()->mType == ESchematicType::EST_Alternate)
	{
		return ((schematic.GetDefaultObject()->mTimeToComplete + 300.f)) - ((schematic.GetDefaultObject()->mTimeToComplete + 300.f) * GetTimeReductionFactor());

	}
	else
	{
		return (schematic.GetDefaultObject()->mTimeToComplete) - (schematic.GetDefaultObject()->mTimeToComplete * GetTimeReductionFactor());

	}
}

float ANogsResearchSubsystem::GetSchematicProgression(TSubclassOf<class UFGSchematic> schematic)
{
	if (!schematic)
		return 600.f;
	if (schematic.GetDefaultObject()->mType == ESchematicType::EST_Alternate)
	{
		return ((schematic.GetDefaultObject()->mTimeToComplete + 300.f) - TimeSpent) - ((schematic.GetDefaultObject()->mTimeToComplete + 300.f) * GetTimeReductionFactor());

	}
	else
	{
		return (schematic.GetDefaultObject()->mTimeToComplete - TimeSpent) - (schematic.GetDefaultObject()->mTimeToComplete * GetTimeReductionFactor());

	}

}

bool ANogsResearchSubsystem::QueSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	if (!SManager->mPurchasedSchematics.Contains(Schematic) && SManager->mAvailableSchematics.Contains(Schematic))
	{
	
		if (Queue.Contains(Schematic))
			return true;

		Queue.Add(Schematic);
		return true;
	}
	return false;
}
bool ANogsResearchSubsystem::RemoveQueSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	if (Queue.Contains(Schematic))
	{
		Queue.Remove(Schematic);
		return true;
	}
	return false;
}
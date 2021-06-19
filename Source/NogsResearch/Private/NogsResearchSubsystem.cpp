

#include "NogsResearchSubsystem.h"
#include "NogsResearchWorldSubsystem.h"
#include "FactoryGame.h"


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
	FString Name = TEXT("BufferInventory");

#if WITH_EDITOR
#else
	if (HasAuthority())
	{
		if (!mBufferInventory)
		{
			mBufferInventory = UFGInventoryLibrary::CreateInventoryComponent(this, *Name.Append(GetName()));
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
	ContentManager = AModContentRegistry::Get(GetWorld());
	Subsystem = Cast< UNogsResearchWorldSubsystem>(GetWorld()->GetSubsystemBase(UNogsResearchWorldSubsystem::StaticClass()));

}


void ANogsResearchSubsystem::ReCalculateSciencePower()
{
	float Power = 0.f;
	for (auto i : Researcher)
	{
		if (i->IsPendingKill())
		{
			Researcher.Remove(i);
			ReCalculateSciencePower();
			break;
		}
		Power += i->SciencePower;
	}
	TotalSciencePower = Power;
}



void ANogsResearchSubsystem::RegisterResearcher(ANogsBuildableResearcher * building)
{
	if (!building)
		return;

	if (!Researcher.Contains(building))
	{
		Researcher.Add(building);
	}
	ReCalculateSciencePower();
}

void ANogsResearchSubsystem::UnRegisterResearcher(ANogsBuildableResearcher * building)
{
	if (!building)
		return;

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

	for (FItemAmount i : cost)
	{
		TArray<TSubclassOf<class UFGItemDescriptor>> lookingFor;
		lookingFor.Add(i.ItemClass);

		TArray<int32> RelevantIndex = Inventory->GetRelevantStackIndexes(lookingFor, 1);

		if (!RelevantIndex.IsValidIndex(0))
			continue;

		FInventoryStack stack;
		Inventory->GetStackFromIndex(RelevantIndex[0], stack);
		if (stack.Item.ItemClass)
		{
			if (stack.Item.ItemClass == i.ItemClass && stack.NumItems > 0)
			{
				int32 remove = FMath::Clamp(stack.NumItems, 0, i.Amount);

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
		default: break;
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


TArray<FItemAmount> ANogsResearchSubsystem::GetMissingItems() const
{
	TArray<FItemAmount> cost;
	const UFGSchematic *  CDO = QueItem.GetDefaultObject();
	if (CDO->mType == ESchematicType::EST_MAM)
	{
		cost = CDO->mCost;
	}
	else if (CDO->mType == ESchematicType::EST_Alternate)
	{
		// cost is usually empty here ..
		// we are adding 2 Hard Drives as additional cost
		// this gets quite complicated later on since we use this function to get only what we dont have already
		cost = CDO->mCost; 
		cost.Add(FItemAmount(AlternateRecipeCostDescriptor, 2));
	}
	else
	{
		return cost = SManager->GetRemainingCostFor(QueItem);
	}
	// check the buffer inventory for items we already have and can subtract from what we need
	for (FItemAmount & j : cost)
	{
		if (j.Amount == 0)
			continue;

		if (j.Amount > 0)
		{
			const int32 LocalAmount = j.Amount - mBufferInventory->GetNumItems(j.ItemClass);
			j.Amount = FMath::Clamp(LocalAmount, 0, 100000);
		}
		
	}
	TArray<FItemAmount> CostOut;
	for (FItemAmount j : cost)
	{
		if (j.Amount > 0)
		{
			CostOut.Add(j);
		}
	}

	return CostOut;
}


void ANogsResearchSubsystem::TickMAMResearch()
{
	if (!Subsystem)
		return;
	if (RManager->IsResearchComplete(QueItem))
	{
		TArray< TSubclassOf< UFGSchematic > > arr;
		int32 ind = 0;
		AFGCharacterPlayer * character = Cast<AFGCharacterPlayer>(GetInstigator());
		while (RManager->ClaimResearchResults(character, QueItem, ind))
		{
			ind++;
		}
		QueItem = nullptr;
		return;
	}

	if (RManager->CanResearchBeInitiated(QueItem))
	{
		const TArray<FItemAmount> Needed = GetMissingItems();
		if (Needed.Num() == 0)
		{
			if (Subsystem->SchematicResearchTreeParents.Contains(QueItem))
			{
				RManager->InitiateResearch(mBufferInventory, QueItem, *Subsystem->SchematicResearchTreeParents.Find(QueItem));
				QueItemLocked = QueItem;
			}
			return;
		}

		if (Researcher.IsValidIndex(Index))
		{
			if (Researcher[Index]->IsPendingKill())
			{
				Researcher.Remove(Researcher[Index]);
				return;
			}


			if (GrabItems(Researcher[Index]->GetStorageInventory()))
			{
				if (!Subsystem->SchematicResearchTreeParents.Contains(QueItem))
				{
					// TODO add logging :Y
					// should never happen tho :I
					
				}
				else
				{
					RManager->InitiateResearch(mBufferInventory, QueItem, *Subsystem->SchematicResearchTreeParents.Find(QueItem));
					QueItemLocked = QueItem;
					ReCalculateSciencePower();
					Queue.Remove(QueItem);
					QueItem = nullptr;
				}
			}
		}
		else
		{
			Index = 0;
			return;
		}
		Index += 1;
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
	if (Researcher.IsValidIndex(Index))
	{
		if (Researcher[Index]->IsPendingKill())
		{
			// dont wanna crash here
			Researcher.Remove(Researcher[Index]);
			return;
		}


		if (!QueItem)
			return;
		const UFGSchematic* CDO = QueItem.GetDefaultObject();
		if (SManager->IsSchematicPaidOff(QueItem) && CDO->mType != ESchematicType::EST_Alternate)
		{
			SManager->LaunchShip();
			QueItemLocked = QueItem;
			ReCalculateSciencePower();
			Queue.Remove(QueItem);
			QueItem = nullptr;
			return;
		}
		else
		{
			if (GrabItems(Researcher[Index]->GetStorageInventory()))
			{
				if (CDO->mType == ESchematicType::EST_Alternate)
				{
					TArray<FItemAmount> cost = GetMissingItems();
					if (cost.Num() == 0)
					{
						cost = CDO->mCost;
						cost.Add(FItemAmount(AlternateRecipeCostDescriptor, 2));

						for (FItemAmount j : cost)
						{
							if (j.Amount == 0)
								continue;

							mBufferInventory->Remove(j.ItemClass, j.Amount);

						}

						SManager->GiveAccessToSchematic(QueItem, false);
						QueItemLocked = QueItem;
						ReCalculateSciencePower();
						Queue.Remove(QueItem);
						QueItem = nullptr;
						return;
					}
				}
				else if (SManager->IsSchematicPaidOff(QueItem))
				{
					SManager->LaunchShip();
					QueItemLocked = QueItem;
					ReCalculateSciencePower();
					Queue.Remove(QueItem);
					QueItem = nullptr;
				}
			}
		}	
	}
	else
	{
		Index = 0;
		return;
	}
	Index += 1;
}

float ANogsResearchSubsystem::GetSchematicDurationAdjusted(TSubclassOf<class UFGSchematic> schematic) const
{
	if (!schematic)
		return 600.f;

	const UFGSchematic* CDO = schematic.GetDefaultObject();
	if (CDO->mType == ESchematicType::EST_Alternate)
	{
		return ((CDO->mTimeToComplete + 300.f)) - ((CDO->mTimeToComplete + 300.f) * GetTimeReductionFactor());

	}
	else
	{
		return (CDO->mTimeToComplete) - (CDO->mTimeToComplete * GetTimeReductionFactor());

	}
}

float ANogsResearchSubsystem::GetSchematicProgression(TSubclassOf<class UFGSchematic> schematic) const
{
	if (!schematic)
		return 600.f;
	const UFGSchematic* CDO = schematic.GetDefaultObject();

	if (CDO->mType == ESchematicType::EST_Alternate)
	{
		return ((CDO->mTimeToComplete + 300.f) - TimeSpent) - ((CDO->mTimeToComplete + 300.f) * GetTimeReductionFactor());

	}
	else
	{
		return (CDO->mTimeToComplete - TimeSpent) - (CDO->mTimeToComplete * GetTimeReductionFactor());

	}

}

bool ANogsResearchSubsystem::QueSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	TArray< TSubclassOf< UFGSchematic >> AllSchematics;
	TArray< TSubclassOf< UFGSchematic >> AllAviSchematics;

	SManager->GetAllPurchasedSchematics(AllSchematics);
	SManager->GetAvailableSchematics(AllAviSchematics);

	if (!AllSchematics.Contains(Schematic) && AllAviSchematics.Contains(Schematic))
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
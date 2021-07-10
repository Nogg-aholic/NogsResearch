

#include "NogsResearchSubsystem.h"
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

	DOREPLIFETIME(ANogsResearchSubsystem, QueItemMAM);
	DOREPLIFETIME(ANogsResearchSubsystem, QueueMAM);
	DOREPLIFETIME(ANogsResearchSubsystem, QueItemLockedMAM);
	DOREPLIFETIME(ANogsResearchSubsystem, TimeSpentMAM);
}


void ANogsResearchSubsystem::BindOnWidgetConstruct(const TSubclassOf<UUserWidget> WidgetClass, FOnWidgetCreated Binding) {
	if (!WidgetClass)
		return;
	UFunction* ConstructFunction = WidgetClass->FindFunctionByName(TEXT("Construct"));
	if (!ConstructFunction || ConstructFunction->IsNative())
	{
		return;
	}
	UBlueprintHookManager* HookManager = GEngine->GetEngineSubsystem<UBlueprintHookManager>();
	HookManager->HookBlueprintFunction(ConstructFunction, [Binding](FBlueprintHookHelper& HookHelper) {
		Binding.ExecuteIfBound(Cast<UUserWidget>(HookHelper.GetContext()));
		}, EPredefinedHookOffset::Return);
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

		mBufferInventory->mItemFilter.BindUFunction(this,"VerifyItem");

	}
#endif
	
	SManager = AFGSchematicManager::Get(GetWorld());
	RManager = AFGResearchManager::Get(GetWorld());
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
			return;
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

ANogsResearchSubsystem * ANogsResearchSubsystem::Get(UObject * WorldContext)
{
	if (!WorldContext)
		return nullptr;
	if (WorldContext->GetWorld())
	{
		
		TArray<AActor *> arr;
		UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), ANogsResearchSubsystem::StaticClass(), arr);
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

bool ANogsResearchSubsystem::GrabItems(UFGInventoryComponent * Inventory, TSubclassOf< class UFGSchematic > Item)
{
	TArray<FItemAmount> cost = GetMissingItems(Item);
	if (cost.Num() == 0)
		return true;

	for (FItemAmount i : cost)
	{
		if (Inventory->HasItems(i.ItemClass, 1))
		{
			const int32 Am = Inventory->GetNumItems(i.ItemClass);
			int32 remove = FMath::Clamp(i.Amount, 0, Am);
			FInventoryStack  stack = FInventoryStack();
			stack.Item.ItemClass = i.ItemClass;
			stack.NumItems = i.Amount;

			if (Item.GetDefaultObject()->mType == ESchematicType::EST_MAM || Item.GetDefaultObject()->mType == ESchematicType::EST_Alternate)
			{
				// only if the inventory is not the same we add it, this will be the case when this is an Alternate Schematic and we want to use the remove as payment of the Cost ( no implementation on CSS side for it) 
				if (mBufferInventory != Inventory && mBufferInventory->HasEnoughSpaceForStack(stack))
				{
					Inventory->Remove(i.ItemClass, mBufferInventory->AddStack(stack));
				}
			}
			else
			{
				// Schematic cost are payed off directly 
				// we dont need to recheck again for what cost is left since this loop is on a descriptor basis and there are no 0 amounts 
				TArray<FItemAmount> payoffArray;
				FItemAmount item = FItemAmount(stack.Item.ItemClass, remove);
				payoffArray.Add(item);
				if(SManager->PayOffOnSchematic(Item, payoffArray))
					Inventory->Remove(i.ItemClass, remove);

				if (SManager->IsSchematicPaidOff(Item))
				{
					return true;
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
		}
		else
		{
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
		
	}
	else if (GetSchematicProgression(QueItemLocked) <= 0)
	{
		QueItemLocked = nullptr;
		TimeSpent = 0.f;
	}
	else
	{
		TimeSpent += dt;
	}

	if (!QueItemLockedMAM)
	{
		if (!QueItemMAM)
		{
			if (QueueMAM.IsValidIndex(0))
			{
				if (QueueMAM[0])
				{
					QueItemMAM = QueueMAM[0];
					const TArray< FItemAmount > Arr = QueItemMAM.GetDefaultObject()->mCost;
					mBufferInventory->Empty();
					mBufferInventory->Resize(FMath::Clamp(Arr.Num(),1,18));
					int32 idx = 0;
					for (const auto i : Arr)
					{
						mBufferInventory->SetAllowedItemOnIndex(idx,i.ItemClass);
						mBufferInventory->AddArbitrarySlotSize(idx,i.Amount);
						idx++;
					}
					QueueMAM.RemoveAt(0);
				}
			}
		}
		else
		{
			switch (QueItemMAM.GetDefaultObject()->mType)
			{
			case ESchematicType::EST_Custom:
			{
				break;
			}
			case ESchematicType::EST_Cheat:
			{
				break;
			}
			case ESchematicType::EST_Tutorial:
			{
				break;
			}
			case ESchematicType::EST_Milestone:
			{
				break;
			}
			case ESchematicType::EST_Alternate:
			{
				break;
			}
			case ESchematicType::EST_Story:
			{
				break;
			}
			case ESchematicType::EST_MAM:
			{
				TickMAMResearch();
				break;
			}
			case ESchematicType::EST_ResourceSink:
			{
				break;
			}
			case ESchematicType::EST_HardDrive:
			{
				break;
			}
			default: break;
			}

		}
	}
	else if (GetSchematicProgression(QueItemLockedMAM) <= 0)
	{
		RManager->OnResearchTimerCompleteAccessor(QueItemLockedMAM);
		QueItemLockedMAM = nullptr;
		TimeSpentMAM = 0.f;
	}
	else
	{
		TimeSpentMAM += dt;
	}
}


TArray<FItemAmount> ANogsResearchSubsystem::GetMissingItems(TSubclassOf< class UFGSchematic > Item) const
{
	TArray<FItemAmount> cost;
	const UFGSchematic *  CDO = Item.GetDefaultObject();
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
		return cost = SManager->GetRemainingCostFor(Item);
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
	if (!RManager)
		return;
	if (RManager->IsResearchComplete(QueItemLockedMAM))
	{
		TArray< TSubclassOf< UFGSchematic > > arr;
		int32 ind = 0;
		AFGCharacterPlayer * character = Cast<AFGCharacterPlayer>(GetInstigator());
		while (RManager->ClaimResearchResults(character, QueItemLockedMAM, ind))
		{
			ind++;
		}
		QueItemLockedMAM = nullptr;
		return;
	}

	if (RManager->CanResearchBeInitiated(QueItemMAM))
	{
		const TArray<FItemAmount> Needed = GetMissingItems(QueItemMAM);
		if (Needed.Num() == 0)
		{
			if (ResearchTreeParents.Contains(QueItemMAM))
			{
				RManager->InitiateResearch(mBufferInventory, QueItemMAM, *ResearchTreeParents.Find(QueItemMAM));
				QueItemLockedMAM = QueItemMAM;
				ReCalculateSciencePower();
				QueueMAM.Remove(QueItemMAM);
				QueItemMAM = nullptr;
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

			if (GrabItems(Researcher[Index]->GetStorageInventory(),QueItemMAM))
			{
				if (!ResearchTreeParents.Contains(QueItemMAM))
				{
					// TODO add logging :Y
					// should never happen tho :I
					
				}
				else
				{
					RManager->InitiateResearch(mBufferInventory, QueItemMAM, *ResearchTreeParents.Find(QueItemMAM));
					QueItemLockedMAM = QueItemMAM;
					ReCalculateSciencePower();
					QueueMAM.Remove(QueItemMAM);
					QueItemMAM = nullptr;
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
	if (Researcher.IsValidIndex(IndexSchematic))
	{
		if (Researcher[IndexSchematic]->IsPendingKill())
		{
			// dont wanna crash here
			Researcher.Remove(Researcher[IndexSchematic]);
			return;
		}

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
			if (GrabItems(Researcher[IndexSchematic]->GetStorageInventory(),QueItem))
			{
				if (CDO->mType == ESchematicType::EST_Alternate)
				{
					TArray<FItemAmount> cost = GetMissingItems(QueItem);
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
		IndexSchematic = 0;
		return;
	}
	IndexSchematic += 1;
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
	else if(CDO->mType == ESchematicType::EST_MAM)
	{
		return (CDO->mTimeToComplete - TimeSpentMAM) - (CDO->mTimeToComplete * GetTimeReductionFactor());
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
		if (Schematic.GetDefaultObject()->mType == ESchematicType::EST_MAM)
		{
			if (QueueMAM.Contains(Schematic))
				return true;

			QueueMAM.Add(Schematic);
			return true;
		}
		else
		{
			if (Queue.Contains(Schematic))
				return true;

			Queue.Add(Schematic);
			return true;
		}	
	}
	return false;
}
bool ANogsResearchSubsystem::RemoveQueSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	if (Schematic.GetDefaultObject()->mType == ESchematicType::EST_MAM)
	{
		if (QueueMAM.Contains(Schematic))
		{
			QueueMAM.Remove(Schematic);
			return true;
		}
	}
	else
	{
		if (Queue.Contains(Schematic))
		{
			Queue.Remove(Schematic);
			return true;
		}
	}
	return false;
}

float ANogsResearchSubsystem::GetTimeReductionFactor() const
{
	if (!ScienceTimeReductionCurve)
		return 0.f;
	return ScienceTimeReductionCurve->GetFloatValue(TotalSciencePower);
}

bool ANogsResearchSubsystem::VerifyItem(TSubclassOf<UFGItemDescriptor> ItemClass, int32 Amount) const
{
	if(QueItemMAM)
		return true;
	else
		return false;
}

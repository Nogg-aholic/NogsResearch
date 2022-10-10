#include "NogsResearchSubsystem.h"
#include "NogsResearch.h"
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
	DOREPLIFETIME(ANogsResearchSubsystem, mBufferInventoryMAM);
	DOREPLIFETIME(ANogsResearchSubsystem, QueueItemHUB);
	DOREPLIFETIME(ANogsResearchSubsystem, QueueHUB);
	DOREPLIFETIME(ANogsResearchSubsystem, QueueItemLockedHUB);
	DOREPLIFETIME(ANogsResearchSubsystem, TimeSpentHUB);

	DOREPLIFETIME(ANogsResearchSubsystem, QueueItemMAM);
	DOREPLIFETIME(ANogsResearchSubsystem, QueueMAM);
	DOREPLIFETIME(ANogsResearchSubsystem, QueueItemLockedMAM);
	DOREPLIFETIME(ANogsResearchSubsystem, TimeSpentMAM);
}

void ANogsResearchSubsystem::BeginPlay()
{
	Super::BeginPlay();

	FString Name = TEXT("BufferInventory");

	// if we dont exclude this for Editor we crash :( TODO doesn't seem to crash any more?
//#if WITH_EDITOR
//#else
	if (HasAuthority()) {
		if (!mBufferInventoryMAM) {
			Name = Name.Append(GetName());
			mBufferInventoryMAM = UFGInventoryLibrary::CreateInventoryComponent(this, *Name);
			UE_LOG(LogNogsResearch, Warning, TEXT("Created MAM buffer inventory with name %s"), *Name);
		}
		else {
			UE_LOG(LogNogsResearch, Display, TEXT("MAM buffer inventory already exists, has name %s"), *mBufferInventoryMAM->GetName());
		}

		if (QueueItemMAM) {
			UpdateMAMBufferFilters(false);
		}
		else if (mBufferInventoryMAM->GetSizeLinear() != 18) {
			UE_LOG(LogNogsResearch, Display, TEXT("No QueueItemMAM so resizing inventory to 18"));
			mBufferInventoryMAM->Resize(18);
		}

		/*for (int32 i = 0; i < mBufferInventoryMAM->mArbitrarySlotSizes.Num(); i++) {
			mBufferInventoryMAM->mArbitrarySlotSizes[i] = 5000;
		}*/

		mBufferInventoryMAM->mItemFilter.BindUFunction(this, "VerifyMAMBufferItemTransfer");
	}
//#endif

	SManager = AFGSchematicManager::Get(GetWorld());
	RManager = AFGResearchManager::Get(GetWorld());
}


void ANogsResearchSubsystem::ReCalculateSciencePower()
{
	float Power = 0.f;
	for (auto i : BuiltResearchers)
	{
		if (!i || i->IsPendingKill())
		{
			BuiltResearchers.Remove(i);
			ReCalculateSciencePower();
			return;
		}
		Power += i->SciencePower;
	}
	TotalSciencePower = Power;
}


void ANogsResearchSubsystem::RegisterResearcher(ANogsBuildableResearcher* building)
{
	if (!building) {
		return;
	}

	if (!BuiltResearchers.Contains(building)) {
		BuiltResearchers.Add(building);
	}
	ReCalculateSciencePower();
}

void ANogsResearchSubsystem::UnRegisterResearcher(ANogsBuildableResearcher* building)
{
	if (!building) {
		return;
	}

	if (BuiltResearchers.Contains(building)) {
		BuiltResearchers.Remove(building);
	}
	ReCalculateSciencePower();
}

ANogsResearchSubsystem* ANogsResearchSubsystem::Get(UObject* WorldContext)
{
	if (!WorldContext) {
		return nullptr;
	}
	if (WorldContext->GetWorld()) {

		TArray<AActor*> arr;
		UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), ANogsResearchSubsystem::StaticClass(), arr);
		if (arr.IsValidIndex(0)) {
			ANogsResearchSubsystem* out = Cast<ANogsResearchSubsystem>(arr[0]);
			return out;
		}
		else {
			return nullptr;
		}
	}
	else {
		return nullptr;
	}
}

bool ANogsResearchSubsystem::GrabItems(UFGInventoryComponent* SourceInventory, TSubclassOf< class UFGSchematic > Schematic)
{
	if (!Schematic || !SourceInventory) {
		return false;
	}

	TArray<FItemAmount> remainingCost = GetMissingItems(Schematic);
	if (remainingCost.Num() == 0) {
		return true;
	}

	for (FItemAmount costEntry : remainingCost) {
		// if we have at least one of the requested remainingCost item
		if (SourceInventory->HasItems(costEntry.ItemClass, 1)) {
			// find out how many we actually have
			int32 quantityPresent = SourceInventory->GetNumItems(costEntry.ItemClass);
			quantityPresent = FMath::Clamp(costEntry.Amount, 0, quantityPresent);

			// set up this Stack for transfer
			FInventoryStack stack = FInventoryStack();
			stack.Item.ItemClass = costEntry.ItemClass;
			stack.NumItems = quantityPresent;

			if (Schematic.GetDefaultObject()->mType == ESchematicType::EST_MAM || Schematic.GetDefaultObject()->mType == ESchematicType::EST_Alternate) {
				// TODO this comment is probably from when the research queue interface was its own building
				if (mBufferInventoryMAM != SourceInventory && mBufferInventoryMAM->HasEnoughSpaceForStack(stack)) {
					const int32 itemsMoved = mBufferInventoryMAM->AddStack(stack);
					SourceInventory->Remove(costEntry.ItemClass, itemsMoved);
				}
			}
			else {
				// Schematic costs are paid off via PayOffOnSchematic
				// we dont need to recheck again for what remainingCost is left since this loop is on a descriptor basis and there are no 0 amounts 
				TArray<FItemAmount> payoffArray;
				FItemAmount submission = FItemAmount(stack.Item.ItemClass, quantityPresent);
				payoffArray.Add(submission);
				if (SManager->PayOffOnSchematic(Schematic, payoffArray)) {
					SourceInventory->Remove(costEntry.ItemClass, quantityPresent);
				}

				if (SManager->IsSchematicPaidOff(Schematic))
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
	if (!HasAuthority()) {
		return;
	}
	if (BuiltResearchers.Contains(nullptr))
	{
		// researchers deleting themselfs are cleared here
		BuiltResearchers.Remove(nullptr);
	}

	if (!QueueItemLockedHUB)
	{
		if (!QueueItemHUB)
		{
			if (QueueHUB.IsValidIndex(0))
			{
				if (QueueHUB[0])
				{
					QueueItemHUB = QueueHUB[0];
					QueueHUB.RemoveAt(0);
					UE_LOG(LogNogsResearch, Display, TEXT("QueueItemHUB became %s"), *QueueItemHUB);
				}
			}
		}
		else
		{
			switch (QueueItemHUB.GetDefaultObject()->mType)
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
				// Handled by MAM section of this function
				break;
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
	else if (GetSchematicProgression(QueueItemLockedHUB) <= 0)
	{
		QueueItemLockedHUB = nullptr;
		TimeSpentHUB = 0.f;
	}
	else
	{
		TimeSpentHUB += dt;
	}

	if (!QueueItemLockedMAM)
	{
		if (!QueueItemMAM)
		{
			if (QueueMAM.IsValidIndex(0))
			{
				if (QueueMAM[0])
				{
					QueueItemMAM = QueueMAM[0];
					QueueMAM.RemoveAt(0);
					UE_LOG(LogNogsResearch, Display, TEXT("QueueItemMAM became %s, so dumping contents and resizing"), *QueueItemMAM);
					UpdateMAMBufferFilters(true);
				}
			}
		}
		else {
			switch (QueueItemMAM.GetDefaultObject()->mType)
			{
			case ESchematicType::EST_Custom:
			{
				// Handled by HUB section
				break;
			}
			case ESchematicType::EST_Cheat:
			{
				// Handled by HUB section
				break;
			}
			case ESchematicType::EST_Tutorial:
			{
				// Handled by HUB section
				break;
			}
			case ESchematicType::EST_Milestone:
			{
				// Handled by HUB section
				break;
			}
			case ESchematicType::EST_Alternate:
			{
				// Handled by HUB section
				break;
			}
			case ESchematicType::EST_Story:
			{
				// Handled by HUB section
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
	else if (GetSchematicProgression(QueueItemLockedMAM) <= 0)
	{
		// RManager->OnResearchTimerCompleteAccessor(QueueItemLockedMAM);
		UE_LOG(LogNogsResearch, Display, TEXT("Completed QueueItemLockedMAM: %s"), *QueueItemLockedMAM);
		RManager->OnResearchTimerComplete(QueueItemLockedMAM);
		QueueItemLockedMAM = nullptr;
		TimeSpentMAM = 0.f;
	}
	else
	{
		TimeSpentMAM += dt;
	}
}

void ANogsResearchSubsystem::UpdateMAMBufferFilters(bool dumpContents)
{
	if (!QueueItemMAM) {
		UE_LOG(LogNogsResearch, Warning, TEXT("Completed QueueItemLockedMAM: %s"), *QueueItemLockedMAM);
		return;
	}
	const TArray< FItemAmount > mamNodeCost = QueueItemMAM.GetDefaultObject()->mCost;
	if (dumpContents) {
		DumpMAMBufferToHUBTerminalGround();
	}
	mBufferInventoryMAM->Resize(FMath::Clamp(mamNodeCost.Num(), 1, 18));
	int32 bufferInvSlotIndex = 0;
	for (const auto costEntry : mamNodeCost)
	{
		mBufferInventoryMAM->SetAllowedItemOnIndex(bufferInvSlotIndex, costEntry.ItemClass);
		mBufferInventoryMAM->AddArbitrarySlotSize(bufferInvSlotIndex, costEntry.Amount);
		bufferInvSlotIndex++;
	}
}

TArray<FItemAmount> ANogsResearchSubsystem::GetMissingItems(TSubclassOf< class UFGSchematic > Schematic) const
{
	TArray<FItemAmount> cost;
	const UFGSchematic* CDO = Schematic.GetDefaultObject();
	if (CDO->mType == ESchematicType::EST_MAM) {
		cost = CDO->mCost;
	}
	else if (CDO->mType == ESchematicType::EST_Alternate) {
		// Cost is usually empty here ..
		// we are adding 2 Hard Drives as additional Cost
		// this gets quite complicated later on since we use this function to get only what we dont have already
		cost = CDO->mCost;
		cost.Add(FItemAmount(AlternateRecipeCostDescriptor, AlternateRecipeCostQuantity));
	}
	else {
		return SManager->GetRemainingCostFor(Schematic);
	}

	// needs further processing
	// check the buffer inventory for items we already have and can subtract from what we need
	for (FItemAmount& itemAmount : cost) {
		if (itemAmount.Amount == 0) {
			continue;
		}

		if (itemAmount.Amount > 0) {
			const int32 LocalAmount = itemAmount.Amount - mBufferInventoryMAM->GetNumItems(itemAmount.ItemClass);
			itemAmount.Amount = FMath::Clamp(LocalAmount, 0, INT_MAX);
		}
	}

	TArray<FItemAmount> CostOut;
	for (FItemAmount itemAmount : cost) {
		if (itemAmount.Amount > 0) {
			CostOut.Add(itemAmount);
		}
	}
	return CostOut;
}


void ANogsResearchSubsystem::TickMAMResearch()
{
	if (!RManager) {
		return;
	}

	if (RManager->IsResearchComplete(QueueItemLockedMAM)) {
		UE_LOG(LogNogsResearch, Display, TEXT("Awarding rewards for MAM research %s"), *QueueItemLockedMAM);
		TArray< TSubclassOf< UFGSchematic > > arr;
		int32 rewardIndex = 0;
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(GetInstigator());
		while (RManager->ClaimResearchResults(character, QueueItemLockedMAM, rewardIndex)) {
			rewardIndex++;
		}
		QueueItemLockedMAM = nullptr;
		return;
	}

	if (RManager->CanResearchBeInitiated(QueueItemMAM))
	{
		const TArray<FItemAmount> Needed = GetMissingItems(QueueItemMAM);
		if (Needed.Num() == 0)
		{
			if (ResearchTreeParents.Contains(QueueItemMAM))
			{
				UE_LOG(LogNogsResearch, Display, TEXT("Initiated MAM research %s with stored items"), *QueueItemMAM);
				RManager->InitiateResearch(mBufferInventoryMAM, QueueItemMAM, *ResearchTreeParents.Find(QueueItemMAM));
				QueueItemLockedMAM = QueueItemMAM;
				ReCalculateSciencePower();
				QueueMAM.Remove(QueueItemMAM);
				QueueItemMAM = nullptr;
			}
			return;
		}

		if (BuiltResearchers.IsValidIndex(BuildingIterateIndexMAM))
		{
			if (BuiltResearchers[BuildingIterateIndexMAM]->IsPendingKill())
			{
				BuiltResearchers.Remove(BuiltResearchers[BuildingIterateIndexMAM]);
				return;
			}

			if (GrabItems(BuiltResearchers[BuildingIterateIndexMAM]->GetStorageInventory(), QueueItemMAM))
			{
				if (!ResearchTreeParents.Contains(QueueItemMAM))
				{
					UE_LOG(LogNogsResearch, Error, TEXT("ResearchTreeParents does not contain QueueItemMAM of %s"), *QueueItemMAM);
				}
				else
				{
					UE_LOG(LogNogsResearch, Display, TEXT("Used items from researcher to initiate MAM research %s"), *QueueItemMAM);
					RManager->InitiateResearch(mBufferInventoryMAM, QueueItemMAM, *ResearchTreeParents.Find(QueueItemMAM));
					QueueItemLockedMAM = QueueItemMAM;
					ReCalculateSciencePower();
					QueueMAM.Remove(QueueItemMAM);
					QueueItemMAM = nullptr;
				}
			}
		}
		else {
			BuildingIterateIndexMAM = 0;
			return;
		}
		BuildingIterateIndexMAM += 1;
	}
}


void ANogsResearchSubsystem::TickSchematicResearch()
{
	if (!QueueItemHUB) {
		return;
	}
	if (SManager->GetActiveSchematic() != QueueItemHUB) {
		UE_LOG(LogNogsResearch, Warning, TEXT("Set the Active Schematic to the QueueItem of %s"), *QueueItemHUB);
		SManager->SetActiveSchematic(QueueItemHUB);
	}
	if (BuiltResearchers.IsValidIndex(BuildingIterateIndexHUB)) {
		if (BuiltResearchers[BuildingIterateIndexHUB]->IsPendingKill())
		{
			// dont wanna crash here
			BuiltResearchers.RemoveAt(BuildingIterateIndexHUB);
			return;
		}

		const UFGSchematic* CDO = QueueItemHUB.GetDefaultObject();
		if (SManager->IsSchematicPaidOff(QueueItemHUB) && CDO->mType != ESchematicType::EST_Alternate)
		{
			SManager->LaunchShip();
			QueueItemLockedHUB = QueueItemHUB;
			ReCalculateSciencePower();
			UE_LOG(LogNogsResearch, Error, TEXT("Launched ship mShipLandTimeStamp: %f mShipLandTimeStampSave: %f"), SManager->mShipLandTimeStamp, SManager->mShipLandTimeStampSave);
			SManager->mShipLandTimeStamp = SManager->mShipLandTimeStamp + 240;
			UE_LOG(LogNogsResearch, Error, TEXT("Bumped Stamp mShipLandTimeStamp: %f mShipLandTimeStampSave: %f"), SManager->mShipLandTimeStamp, SManager->mShipLandTimeStampSave);
			QueueHUB.Remove(QueueItemHUB);
			QueueItemHUB = nullptr;
			return;
		}
		else
		{
			// Not paid off yet (or alternate) so grab stuff
			if (GrabItems(BuiltResearchers[BuildingIterateIndexHUB]->GetStorageInventory(), QueueItemHUB))
			{
				if (CDO->mType == ESchematicType::EST_Alternate)
				{
					TArray<FItemAmount> cost = GetMissingItems(QueueItemHUB);
					if (cost.Num() == 0)
					{
						cost = CDO->mCost;
						cost.Add(FItemAmount(AlternateRecipeCostDescriptor, AlternateRecipeCostQuantity));

						for (const FItemAmount j : cost)
						{
							if (j.Amount == 0)
								continue;
							mBufferInventoryMAM->Remove(j.ItemClass, j.Amount);
						}

						UE_LOG(LogNogsResearch, Error, TEXT("Grab Items Succeed EST_Alternate route"));
						SManager->GiveAccessToSchematic(QueueItemHUB, false);
						QueueItemLockedHUB = QueueItemHUB;
						ReCalculateSciencePower();
						QueueHUB.Remove(QueueItemHUB);
						QueueItemHUB = nullptr;
						return;
					}
				}
				else if (SManager->IsSchematicPaidOff(QueueItemHUB))
				{
					UE_LOG(LogNogsResearch, Error, TEXT("Grab Items Succeed Not EST_Alternate route"));
					SManager->LaunchShip();
					QueueItemLockedHUB = QueueItemHUB;
					ReCalculateSciencePower();
					QueueHUB.Remove(QueueItemHUB);
					QueueItemHUB = nullptr;
				}
			}
		}
	}
	else
	{
		BuildingIterateIndexHUB = 0;
		return;
	}
	BuildingIterateIndexHUB += 1;
}

float ANogsResearchSubsystem::GetSchematicDurationAdjusted(TSubclassOf<class UFGSchematic> schematic) const
{
	if (!schematic) {
		return 600.f;
	}

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
	if (!schematic) {
		return 600.f;
	}
	const UFGSchematic* CDO = schematic.GetDefaultObject();

	if (CDO->mType == ESchematicType::EST_Alternate)
	{
		return ((CDO->mTimeToComplete + 300.f) - TimeSpentHUB) - ((CDO->mTimeToComplete + 300.f) * GetTimeReductionFactor());
	}
	else if (CDO->mType == ESchematicType::EST_MAM)
	{
		return (CDO->mTimeToComplete - TimeSpentMAM) - (CDO->mTimeToComplete * GetTimeReductionFactor());
	}
	else
	{
		return (CDO->mTimeToComplete - TimeSpentHUB) - (CDO->mTimeToComplete * GetTimeReductionFactor());
	}
}

bool ANogsResearchSubsystem::QueueSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	TArray< TSubclassOf< UFGSchematic >> AllSchematics;
	TArray< TSubclassOf< UFGSchematic >> AllAviSchematics;

	SManager->GetAllPurchasedSchematics(AllSchematics);
	SManager->GetAvailableSchematics(AllAviSchematics);

	if (!AllSchematics.Contains(Schematic) && AllAviSchematics.Contains(Schematic))
	{
		if (Schematic.GetDefaultObject()->mType == ESchematicType::EST_MAM)
		{
			UE_LOG(LogNogsResearch, Display, TEXT("Added %s to MAM Queue"), *Schematic);
			if (QueueMAM.Contains(Schematic))
				return true;

			QueueMAM.Add(Schematic);
			return true;
		}
		else
		{
			UE_LOG(LogNogsResearch, Display, TEXT("Added %s to HUB Queue"), *Schematic);
			if (QueueHUB.Contains(Schematic))
				return true;

			QueueHUB.Add(Schematic);
			return true;
		}
	}
	return false;
}

bool ANogsResearchSubsystem::RemoveQueueSchematic(TSubclassOf<class UFGSchematic> Schematic)
{
	if (Schematic.GetDefaultObject()->mType == ESchematicType::EST_MAM)
	{
		if (QueueMAM.Contains(Schematic))
		{
			UE_LOG(LogNogsResearch, Display, TEXT("Removed %s from MAM Queue"), *Schematic);
			QueueMAM.Remove(Schematic);
			return true;
		}
	}
	else
	{
		if (QueueHUB.Contains(Schematic))
		{
			UE_LOG(LogNogsResearch, Display, TEXT("Removed %s from HUB Queue"), *Schematic);
			QueueHUB.Remove(Schematic);
			return true;
		}
	}
	UE_LOG(LogNogsResearch, Warning, TEXT("Failed to dequeue %s"), *Schematic);
	return false;
}

float ANogsResearchSubsystem::GetTimeReductionFactor() const
{
	if (!ScienceTimeReductionCurve) {
		return 0.f;
	}
	return ScienceTimeReductionCurve->GetFloatValue(TotalSciencePower);
}

bool ANogsResearchSubsystem::VerifyMAMBufferItemTransfer(TSubclassOf<UFGItemDescriptor> ItemClass, int32 Amount) const
{
	if (QueueItemMAM) {
		return true;
	}
	else {
		return false;
	}
}

void ANogsResearchSubsystem::DumpMAMBufferToHUBTerminalGround_Implementation() {
	UE_LOG(LogNogsResearch, Error, TEXT("No Cpp implementation yet for DumpMAMBufferToHUBTerminalGround"));
}

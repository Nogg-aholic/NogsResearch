

#include "Buildable/NogsBuildableResearcher.h"

#include "FGPipeConnectionComponent.h"
#include "NogsResearchSubsystem.h"
#include "FGPowerInfoComponent.h"

#include "NogsResearch.h"

ANogsBuildableResearcher::ANogsBuildableResearcher() : Super() {
	this->mPowerInfoClass = UFGPowerInfoComponent::StaticClass();
	this->mInventorySizeX = 1;
	this->mInventorySizeY = 1;
	this->mPowerConsumptionExponent = 1.60000002384186;
	this->mMinimumProducingTime = 2;
	this->mMinimumStoppedTime = 5;
	// this->mNumCyclesForProductivity = 20; // U7 removed
	this->mPendingPotential = 1;
	this->mMinPotential = 0.00999999977648258;
	this->mMaxPotential = 1;
	this->mMaxPotentialIncreasePerCrystal = 0.5;
	this->mFluidStackSizeDefault = EStackSize::SS_FLUID;
	this->mSignificanceRange = 18000;
	this->MaxRenderDistance = -1;
	this->mFactoryTickFunction.TickGroup = TG_PrePhysics; this->mFactoryTickFunction.EndTickGroup = TG_PrePhysics; this->mFactoryTickFunction.bTickEvenWhenPaused = false; this->mFactoryTickFunction.bCanEverTick = true; this->mFactoryTickFunction.bStartWithTickEnabled = true; this->mFactoryTickFunction.bAllowTickOnDedicatedServer = true; this->mFactoryTickFunction.TickInterval = 0;
	// this->mPrimaryColor.R = -1; this->mPrimaryColor.G = -1; this->mPrimaryColor.B = -1; this->mPrimaryColor.A = 1;
	// this->mSecondaryColor.R = -1; this->mSecondaryColor.G = -1; this->mSecondaryColor.B = -1; this->mSecondaryColor.A = 1;
	// TOOD u8 removed this, is it sill fine?
	// this->mDismantleEffectClassName = FSoftClassPath("/Game/FactoryGame/Buildable/Factory/-Shared/BP_MaterialEffect_Dismantle.BP_MaterialEffect_Dismantle_C");
	// this->mBuildEffectClassName = FSoftClassPath("/Game/FactoryGame/Buildable/Factory/-Shared/BP_MaterialEffect_Build.BP_MaterialEffect_Build_C");
	// this->mHighlightParticleClassName = FSoftClassPath("/Game/FactoryGame/Buildable/-Shared/Particle/NewBuildingPing.NewBuildingPing_C");
	this->bReplicates = true;
	this->NetCullDistanceSquared = 5624999936;
	this->Registered = false;
	this->SManager = nullptr;
}


void ANogsBuildableResearcher::BeginPlay()
{
	// for some reason we need to set the Power Info here again otherwise power didnt work
	Super::BeginPlay();
	if (!mPowerInfo) {
		mPowerInfo = Cast< UFGPowerInfoComponent>(GetComponentByClass(UFGPowerInfoComponent::StaticClass()));
	}

	mPowerInfo->OnHasPowerChanged.BindUFunction(this, "CheckPower");
	SManager = ANogsResearchSubsystem::Get(this->GetWorld());
	if (!HasAuthority()) {
		return;
	}

	// we set all slots to our custom Slot Size
	for (int32 i = 0; i < GetStorageInventory()->GetSizeLinear(); i++) {
		GetStorageInventory()->AddArbitrarySlotSize(i, ArbitraryStackSize);
	}

	FOR_EACH_PIPE_INLINE_COMPONENTS(connection) {
		if (connection->GetPipeConnectionType() == EPipeConnectionType::PCT_CONSUMER)
		{
			connection->SetInventory(GetStorageInventory());
			connection->SetInventoryAccessIndex(0);
			Pipe = connection;
			GetStorageInventory()->AddArbitrarySlotSize(0, 5000 * mFluidStackSizeMultiplier);
		}
	}

}

void ANogsBuildableResearcher::CheckPower()
{
	// const auto hasPower = HasPower(); // As of U8 this seems to be one tick behind, so use Factory instead
	const auto factoryHasPower = Factory_HasPower();
	if (factoryHasPower) {
		if (!Registered) {
			if (SManager) {
				UE_LOG(LogNogsResearchCpp, Warning, TEXT("Registering researcher %s"), *GetName());
				Registered = true;
				SManager->RegisterResearcher(this);
				ProductionStateChanged();
			}
		}
	}
	else {
		if (Registered) {
			if (SManager) {
				UE_LOG(LogNogsResearchCpp, Warning, TEXT("UnRegistering researcher %s"), *GetName());
				Registered = false;
				SManager->UnRegisterResearcher(this);
				ProductionStateChanged();
			}
		}
	}
}


bool ANogsBuildableResearcher::Factory_HasPower() const
{
	return Super::Factory_HasPower();
	// U8 seems to have made this obsolete
	//if (GetPowerInfo()) {
	//	// 2 lines so the debugger can be attached and preview the result
	//	auto power = GetPowerInfo()->HasPower();
	//	return power;
	//}
	//return false;
}

void ANogsBuildableResearcher::Factory_Tick(float dt)
{
	Super::Factory_Tick(dt);
	if (HasAuthority() && IsValid(this) && Pipe && GetStorageInventory()) {
		if (Factory_HasPower()) {
			if (Pipe->IsConnected()) {
				if (GetStorageInventory() && GetStorageInventory()->IsSomethingOnIndex(0)) {
					FInventoryStack CurrentItem; GetStorageInventory()->GetStackFromIndex(0, CurrentItem);
					if (CurrentItem.Item.GetItemClass()) {
						FInventoryStack Stack;
						Pipe->Factory_PullPipeInput(dt, Stack, CurrentItem.Item.GetItemClass(), FMath::Clamp((5000.f * mFluidStackSizeMultiplier) - CurrentItem.NumItems, 0.f, 300.f * dt));
						if (Stack.HasItems()) {
							GetStorageInventory()->AddStackToIndex(0, Stack);
						}
					}
				}
				else {
					FInventoryStack Stack;
					Pipe->Factory_PullPipeInput(dt, Stack, nullptr, FMath::Clamp(5000.f * mFluidStackSizeMultiplier, 0.f, 300.f * dt));
					if (Stack.HasItems()) {
						GetStorageInventory()->AddStackToIndex(0, Stack);
					}
				}
			}
		}
	}
}





#include "NogsBuildableResearcher.h"
#include "NogsResearchSubsystem.h"
#include "FGPowerInfoComponent.h"

ANogsBuildableResearcher::ANogsBuildableResearcher() : Super() {
	this->mPowerInfoClass = UFGPowerInfoComponent::StaticClass();
	this->mInventorySizeX = 1;
	this->mInventorySizeY = 1;
	this->mPowerConsumptionExponent = 1.60000002384186;
	this->mMinimumProducingTime = 2;
	this->mMinimumStoppedTime = 5;
	this->mNumCyclesForProductivity = 20;
	this->mCurrentPotential = 1;
	this->mPendingPotential = 1;
	this->mMinPotential = 0.00999999977648258;
	this->mMaxPotential = 1;
	this->mMaxPotentialIncreasePerCrystal = 0.5;
	this->mFluidStackSizeDefault = EStackSize::SS_FLUID;
	this->mSignificanceRange = 18000;
	this->MaxRenderDistance = -1;
	this->mFactoryTickFunction.TickGroup = TG_PrePhysics; this->mFactoryTickFunction.EndTickGroup = TG_PrePhysics; this->mFactoryTickFunction.bTickEvenWhenPaused = false; this->mFactoryTickFunction.bCanEverTick = true; this->mFactoryTickFunction.bStartWithTickEnabled = true; this->mFactoryTickFunction.bAllowTickOnDedicatedServer = true; this->mFactoryTickFunction.TickInterval = 0;
	this->mPrimaryColor.R = -1; this->mPrimaryColor.G = -1; this->mPrimaryColor.B = -1; this->mPrimaryColor.A = 1;
	this->mSecondaryColor.R = -1; this->mSecondaryColor.G = -1; this->mSecondaryColor.B = -1; this->mSecondaryColor.A = 1;
	this->mDismantleEffectClassName = FSoftClassPath("/Game/FactoryGame/Buildable/Factory/-Shared/BP_MaterialEffect_Dismantle.BP_MaterialEffect_Dismantle_C");
	this->mBuildEffectClassName = FSoftClassPath("/Game/FactoryGame/Buildable/Factory/-Shared/BP_MaterialEffect_Build.BP_MaterialEffect_Build_C");
	this->mHighlightParticleClassName = FSoftClassPath("/Game/FactoryGame/Buildable/-Shared/Particle/NewBuildingPing.NewBuildingPing_C");
	this->bReplicates = true;
	this->NetCullDistanceSquared = 5624999936;
}


void ANogsBuildableResearcher::BeginPlay()
{
	// for some reason we need to set the Power Info here again otherwise power didnt work
	Super::BeginPlay();
	mPowerInfo = Cast< UFGPowerInfoComponent>(GetComponentByClass(UFGPowerInfoComponent::StaticClass()));
	SManager = ANogsResearchSubsystem::Get(this->GetWorld());
	if (!HasAuthority())
		return;

	// we set all slots to our custom Slot Size
	for (int32 i = 0; i < GetStorageInventory()->mArbitrarySlotSizes.Num(); i++)
	{
		GetStorageInventory()->mArbitrarySlotSizes[i] = 5000;
	}
}

void ANogsBuildableResearcher::Factory_Tick(float dt)
{
	// nothing special here we just register or unregister ourselfs depending on if we have power
	Super::Factory_Tick(dt);
	if (HasPower())
	{
		if (!Registered)
		{
			if (SManager)
			{
				Registered = true;
				ProductionStateChanged();
				Cast<ANogsResearchSubsystem>(SManager)->RegisterResearcher(this);
			}
		}
	}
	else
	{
		if (Registered)
		{
			if (SManager)
			{
				Registered = false;
				Cast<ANogsResearchSubsystem>(SManager)->UnRegisterResearcher(this);
				ProductionStateChanged();
			}
		}
	}
}


bool ANogsBuildableResearcher::HasPower() const
{
	if(GetPowerInfo())
		return GetPowerInfo()->HasPower();
	return false;
}

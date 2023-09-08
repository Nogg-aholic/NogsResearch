#pragma once

#include "CoreMinimal.h"

#include "FGPipeConnectionComponent.h"
#include "Buildables/FGBuildableStorage.h"
#include "NogsBuildableResearcher.generated.h"


class ANogsResearchSubsystem;
/**
 * 
 */
UCLASS()
class NOGSRESEARCH_API ANogsBuildableResearcher : public AFGBuildableStorage
{
	GENERATED_BODY()
		ANogsBuildableResearcher();

	virtual void BeginPlay() override;
public:
	
	UFUNCTION(BlueprintCallable)
	void CheckPower();
		
	virtual bool Factory_HasPower() const override;

	virtual void Factory_Tick(float dt) override;


	UFUNCTION(BlueprintImplementableEvent)
		void ProductionStateChanged();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SciencePower;

	UPROPERTY(BlueprintReadWrite)
		bool Registered;

	UPROPERTY()
		ANogsResearchSubsystem* SManager;

	UPROPERTY(BlueprintReadOnly)
		UFGPipeConnectionComponent* Pipe;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 ArbitraryStackSize = 500;

	virtual bool CanProduce_Implementation() const override;
};

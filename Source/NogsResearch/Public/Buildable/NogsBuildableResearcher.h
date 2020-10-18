

#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableStorage.h"
#include "NogsBuildableResearcher.generated.h"

/**
 * 
 */
UCLASS()
class NOGSRESEARCH_API ANogsBuildableResearcher : public AFGBuildableStorage
{
	GENERATED_BODY()
		ANogsBuildableResearcher();
public:


	virtual void BeginPlay() override;
	virtual void Factory_Tick(float dt) override;

	virtual bool HasPower() const override;

	UFUNCTION(BlueprintImplementableEvent)
		void ProductionStateChanged();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SciencePower = 0.f;

	UPROPERTY(BlueprintReadWrite)
		bool Registered;

	AActor * SManager;

};

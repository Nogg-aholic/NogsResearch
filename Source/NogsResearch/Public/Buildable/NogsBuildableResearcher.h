

#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION(BlueprintImplementableEvent)
		void ProductionStateChanged();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float SciencePower;

	UPROPERTY(BlueprintReadWrite)
		bool Registered;

	UPROPERTY()
	ANogsResearchSubsystem* SManager;;

};

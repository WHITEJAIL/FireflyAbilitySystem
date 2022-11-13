// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FireflyAbilityManagerComponent.generated.h"

class UFireflyAbility;

/** 技能管理器组件 */
UCLASS( ClassGroup=(FireflyAbility), meta=(BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyAbilityManagerComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

#pragma region Override

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion


#pragma region Management

public:
	/** 获取该技能管理器被赋予的所有技能 */
	UFUNCTION()
	FORCEINLINE TArray<UFireflyAbility*> GetGrantedAbilities() const { return GrantedAbilities; }

protected:
	/** 技能管理器被赋予的技能 */
	UPROPERTY()
	TArray<UFireflyAbility*> GrantedAbilities;

#pragma endregion


#pragma region Requirement
public:
	/** 获取该技能管理器被赋予的所有技能 */
	UFUNCTION()
	FORCEINLINE FGameplayTagContainer GetCarryingGameplayTags() const { return CarryingGameplayTags; }

protected:
	/** 技能管理器携带的所有标签Tags */
	UPROPERTY()
	FGameplayTagContainer CarryingGameplayTags;

#pragma endregion

};

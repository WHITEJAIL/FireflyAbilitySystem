// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbility_InputBased.h"
#include "Ability/FireflyAbilityManagerComponent.h"
#include "FireflyPlayerAbilityManagerComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;

USTRUCT()
struct FFireflyAbilitiesBoundToInput
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<TSubclassOf<UFireflyAbility_InputBased>> Abilities;

	FFireflyAbilitiesBoundToInput() : Abilities(TArray<TSubclassOf<UFireflyAbility_InputBased>>{})
	{}

	FFireflyAbilitiesBoundToInput(TArray<TSubclassOf<UFireflyAbility_InputBased>> InAbilities)
		: Abilities(InAbilities)
	{}

	FORCEINLINE bool operator==(const FFireflyAbilitiesBoundToInput& Other)
	{
		return Abilities == Other.Abilities;
	}
	
};

/** 玩家技能管理器组件，主要包含输入绑定 */
UCLASS( ClassGroup = (FireflyAbility), meta = (BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyPlayerAbilityManagerComponent : public UFireflyAbilityManagerComponent
{
	GENERATED_UCLASS_BODY()

#pragma region InputBinding

protected:
	/** 从组件拥有者身上获取增强输入组件 */
	UFUNCTION()
	FORCEINLINE UEnhancedInputComponent* GetEnhancedInputComponentFromOwner() const;

public:
	/** 将技能与输入绑定，技能需要存在于技能管理器中，输入也应当有效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void BindAbilityToInput(TSubclassOf<UFireflyAbility_InputBased> AbilityToBind, UInputAction* InputToBind);

	/** 将技能与输入绑定，技能需要存在于技能管理器中，输入也应当有效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void UnbindAbilityWithInput(TSubclassOf<UFireflyAbility_InputBased> AbilityToUnbind, UInputAction* InputToUnbind);

protected:
	/** 所有和输入绑定了的技能 */
	UPROPERTY()
	TMap<UInputAction*, FFireflyAbilitiesBoundToInput> AbilitiesInputBound;

#pragma endregion
	
};

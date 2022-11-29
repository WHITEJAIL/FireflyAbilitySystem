// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbility_InputBased.h"
#include "Ability/FireflyAbilityManagerComponent.h"
#include "FireflyPlayerAbilityManagerComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;

/** 输入和技能绑定的数据 */
USTRUCT()
struct FFireflyAbilitiesBoundToInput
{
	GENERATED_USTRUCT_BODY()

	/** 输入绑定的所有技能 */
	UPROPERTY()
	TArray<TSubclassOf<UFireflyAbility_InputBased>> Abilities;

	/** 输入事件句柄：开始 */
	UPROPERTY()
	uint32 HandleStarted = -1;

	/** 输入事件句柄：执行中 */
	UPROPERTY()
	uint32 HandleOngoing = -1;

	/** 输入事件句柄：取消 */
	UPROPERTY()
	uint32 HandleCanceled = -1;

	/** 输入事件句柄：触发 */
	UPROPERTY()
	uint32 HandleTriggered = -1;

	/** 输入事件句柄：完成 */
	UPROPERTY()
	uint32 HandleCompleted = -1;

	FFireflyAbilitiesBoundToInput() : Abilities(TArray<TSubclassOf<UFireflyAbility_InputBased>>{})
	{}

	FORCEINLINE bool operator==(const FFireflyAbilitiesBoundToInput& Other)
	{
		return Abilities == Other.Abilities
			&& HandleStarted == Other.HandleStarted
			&& HandleOngoing == Other.HandleOngoing
			&& HandleCanceled == Other.HandleCanceled
			&& HandleTriggered == Other.HandleTriggered
			&& HandleCompleted == Other.HandleCompleted;
	}
	
};

/** 玩家技能管理器组件，主要包含输入绑定 */
UCLASS( ClassGroup = (FireflyAbilitySystem), meta = (BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyPlayerAbilityManagerComponent : public UFireflyAbilityManagerComponent
{
	GENERATED_UCLASS_BODY()

#pragma region Basic

protected:
	/** 从组件拥有者身上获取增强输入组件 */
	UFUNCTION()
	FORCEINLINE UEnhancedInputComponent* GetEnhancedInputComponentFromOwner() const;

	/** 组件拥有者是否拥有本地控制权限 */
	UFUNCTION()
	FORCEINLINE bool IsOwnerLocallyControlled() const;

#pragma endregion


#pragma region InputBinding

public:
	/** 将技能与输入绑定，绑定操作应该在本地客户端执行，技能需要存在于技能管理器中，输入也应当有效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void BindAbilityToInput(TSubclassOf<UFireflyAbility_InputBased> AbilityToBind, UInputAction* InputToBind);

	/** 将技能与输入解绑，解绑操作应该在本地客户端执行，技能需要存在于技能管理器中，输入也应当有效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void UnbindAbilityWithInput(TSubclassOf<UFireflyAbility_InputBased> AbilityToUnbind, UInputAction* InputToUnbind);

protected:
	/** 所有和输入绑定了的技能 */
	UPROPERTY()
	TMap<UInputAction*, FFireflyAbilitiesBoundToInput> AbilitiesInputBound;

#pragma endregion


#pragma region InputEvent

protected:
	/** 组件管理的输入事件触发：开始 */
	UFUNCTION()
	virtual void OnAbilityInputActionStarted(UInputAction* Input);

	/** 组件管理的输入事件触发：等待触发 */
	UFUNCTION()
	virtual void OnAbilityInputActionOngoing(UInputAction* Input);

	/** 组件管理的输入事件触发：取消 */
	UFUNCTION()
	virtual void OnAbilityInputActionCanceled(UInputAction* Input);

	/** 组件管理的输入事件触发：触发 */
	UFUNCTION()
	virtual void OnAbilityInputActionTriggered(UInputAction* Input);

	/** 组件管理的输入事件触发：完成 */
	UFUNCTION()
	virtual void OnAbilityInputActionCompleted(UInputAction* Input);

#pragma endregion
	
};

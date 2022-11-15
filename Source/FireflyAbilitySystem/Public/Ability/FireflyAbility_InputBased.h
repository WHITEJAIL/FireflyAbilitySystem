// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/FireflyAbility.h"
#include "FireflyAbility_InputBased.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class UFireflyPlayerAbilityManagerComponent;

/** 可被绑定的输入事件触发的技能 */
UCLASS(Blueprintable, BlueprintType)
class FIREFLYABILITYSYSTEM_API UFireflyAbility_InputBased : public UFireflyAbility
{
	GENERATED_UCLASS_BODY()

#pragma region Execution

public:
	/** 是否可激活技能 */
	virtual bool CanActivateAbility() const override;

protected:
	/** 该技能的激活需要的正在执行的技能 */
	UPROPERTY(EditDefaultsOnly, Category = "FireflyAbilitySystem|Ability")
	TArray<TSubclassOf<UFireflyAbility>> RequiredActivatingAbilities;

#pragma endregion

#pragma region InputBinding

protected:
	/** 技能绑定输入 */
	UFUNCTION()
	void BindToInput(UEnhancedInputComponent* EnhancedInput, UInputAction* InputToBind);

	/** 技能与输入解绑 */
	UFUNCTION()
	void UnbindWithInput(UEnhancedInputComponent* EnhancedInput, UInputAction* InputToUnbind);

	/** 输入事件：开始 */
	UFUNCTION()
	virtual void OnAbilityInputStarted();

	/** 蓝图输入事件：开始 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability")
	void ReceiveOnAbilityInputStarted();

	/** 输入事件：执行中 */
	UFUNCTION()
	virtual void OnAbilityInputOngoing();

	/** 蓝图输入事件：执行中 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability")
	void ReceiveOnAbilityInputOngoing();

	/** 输入事件：取消 */
	UFUNCTION()
	virtual void OnAbilityInputCanceled();

	/** 蓝图输入事件：取消 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability")
	void ReceiveOnAbilityInputCanceled();

	/** 输入事件：触发 */
	UFUNCTION()
	virtual void OnAbilityInputTriggered();

	/** 蓝图输入事件：触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability")
	void ReceiveOnAbilityInputTriggered();

	/** 输入事件：完成 */
	UFUNCTION()
	virtual void OnAbilityInputCompleted();

	/** 蓝图输入事件：完成 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability")
	void ReceiveOnAbilityInputCompleted();

protected:
	friend class UFireflyPlayerAbilityManagerComponent;

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

#pragma endregion
	
};

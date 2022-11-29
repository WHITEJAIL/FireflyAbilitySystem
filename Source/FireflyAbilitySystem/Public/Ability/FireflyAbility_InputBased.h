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

#pragma region InputBinding

protected:
	/** 输入事件：开始 */
	UFUNCTION()
	virtual void OnAbilityInputStarted();

	/** 本地客户端通知服务端执行输入事件：开始 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputStarted();

	/** 蓝图输入事件：开始 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Started"))
	void ReceiveOnAbilityInputStarted();

	/** 输入事件：执行中 */
	UFUNCTION()
	virtual void OnAbilityInputOngoing();

	/** 本地客户端通知服务端执行输入事件：执行中 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputOngoing();

	/** 蓝图输入事件：执行中 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Ongoing"))
	void ReceiveOnAbilityInputOngoing();

	/** 输入事件：取消 */
	UFUNCTION()
	virtual void OnAbilityInputCanceled();

	/** 本地客户端通知服务端执行输入事件：取消 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputCanceled();

	/** 蓝图输入事件：取消 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Canceled"))
	void ReceiveOnAbilityInputCanceled();

	/** 输入事件：触发 */
	UFUNCTION()
	virtual void OnAbilityInputTriggered();

	/** 本地客户端通知服务端执行输入事件：触发 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputTriggered();

	/** 蓝图输入事件：触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Triggered"))
	void ReceiveOnAbilityInputTriggered();

	/** 输入事件：完成 */
	UFUNCTION()
	virtual void OnAbilityInputCompleted();

	/** 本地客户端通知服务端执行输入事件：完成 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputCompleted();

	/** 蓝图输入事件：完成 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Completed"))
	void ReceiveOnAbilityInputCompleted();

protected:
	friend class UFireflyPlayerAbilityManagerComponent;

	/** 是否在输入事件Triggered时激活技能，如为false，则默认在输入事件Started时激活技能 */
	UPROPERTY(EditDefaultsOnly, Category = "Input Activation")
	bool bActivateOnTriggered = false;

#pragma endregion
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbilitySystemTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "FireflyAbility.generated.h"

class UFireflyAbilitySystemComponent;

/** 技能 */
UCLASS(Blueprintable, BlueprintType)
class FIREFLYABILITYSYSTEM_API UFireflyAbility : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Override // 基类函数重载

public:
	virtual UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;

	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

#pragma endregion


#pragma region Basic // 基础函数
	
protected:
	/** 获取技能所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	AActor* GetOwnerActor() const;

	/** 获取技能所属的拥有者的网络权限 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	ENetRole GetOwnerRole() const;

	/** 获取技能所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	UFireflyAbilitySystemComponent* GetOwnerManager() const;

	/** 管理器的拥有者是否拥有权威权限 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool HasAuthority() const;

	/** 组件拥有者是否拥有本地控制权限 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool IsLocallyControlled() const;

protected:
	friend UFireflyAbilitySystemComponent;

	/** 技能的唯一ID标识 */
	UPROPERTY()
	FName AbilityID;

#pragma endregion


#pragma region Granting // 技能赋予

protected:
	/**当技能被赋予时执行的函数，可以理解为技能的运行时构造函数*/
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	void OnAbilityGranted();

protected:
	/** 标识该技能是否应该在某次执行结束后从技能管理器上移除 */
	UPROPERTY()
	bool bRemoveOnEndedExecution = false;

#pragma endregion


#pragma region Execution // 技能执行

protected:
	/** 激活技能 */
	UFUNCTION()
	virtual void ActivateAbility();

	/** 蓝图端的技能激活时的逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "Activate Ability"))
	void ReceiveActivateAbility();

	/** 结束技能，内部执行 */
	UFUNCTION()
	virtual void EndAbilityInternal();

	/** 结束技能 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	virtual void EndAbility();

	/** 本地客户端通知服务端结束技能 */
	UFUNCTION(Server, Reliable)
	void Server_EndAbility();

	/** 服务端通知本地客户端结束技能 */
	UFUNCTION(Client, Reliable)
	void Client_EndAbility();

	/** 取消技能，内部执行 */
	UFUNCTION()
	virtual void CancelAbilityInternal();

	/** 取消技能 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	virtual void CancelAbility();

	/** 本地客户端通知服务端取消技能 */
	UFUNCTION(Server, Reliable)
	void Server_CancelAbility();

	/** 服务端通知本地客户端取消技能 */
	UFUNCTION(Client, Reliable)
	void Client_CancelAbility();

	/** 蓝图端的技能结束时执行的逻辑，分为自动结束和取消结束两种状态 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "End Ability"))
	void ReceiveEndAbility(bool bWasCanceled);	

protected:
	/** 该技能是否处于激活状态 */
	UPROPERTY()
	bool bIsActivating = false;

	/** 该技能是否在被赋予后立即执行 */
	UPROPERTY(EditDefaultsOnly, Category = Execution)
	bool bActivateOnGranted = false;

#pragma endregion


#pragma region CostAndCooldown // 消耗和冷却

protected:
	/** 检测技能的消耗是否可执行 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	bool CheckAbilityCost() const;

	/** 申请执行技能的消耗 */
	UFUNCTION()
	virtual void ApplyAbilityCost();

	/** 检测技能的冷却是否可执行 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	bool CheckAbilityCooldown() const;

	/** 申请执行技能的冷却 */
	UFUNCTION()
	virtual void ApplyAbilityCooldown();

	/** 单独执行技能的消耗 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	void CommitAbilityCost();

	/** 本地客户端通知服务端单独执行机能的消耗 */
	UFUNCTION(Server, Reliable)
	void Server_CommitAbilityCost();

	/** 单独执行技能的冷却 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	void CommitAbilityCooldown();

	/** 本地客户端通知服务端单独执行机能的冷却 */
	UFUNCTION(Server, Reliable)
	void Server_CommitAbilityCooldown();

	/** 执行技能的消耗和冷却 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	void CommitAbility();

	/** 设置技能的冷却时间 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	void SetCooldownTime(float NewCooldownTime);

	/** 获取技能的冷却时间 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetCooldownTime() const { return CooldownTime; }

	/** 设置技能的冷却标签 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	void SetCooldownTags(FGameplayTagContainer NewCooldownTags);

	/** 获取技能的冷却标签 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	FORCEINLINE FGameplayTagContainer GetCooldownTags() const { return CooldownTags; }

	/** 设置技能的消耗设置 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	void SetCostSettings(TArray<FFireflyEffectModifierData> NewCostSettings);

	/** 获取技能的消耗设置 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	TArray<FFireflyEffectModifierData> GetCostSettings() const { return CostSettings; }

protected:	
	/** 技能的消耗属性设置 */
	UPROPERTY(EditDefaultsOnly, Category = Cost)
	TArray<FFireflyEffectModifierData> CostSettings;

	/** 技能的冷却时间设置 */
	UPROPERTY(EditDefaultsOnly, Category = Cooldown)
	float CooldownTime = 0.f;

	/** 技能的冷却时间使用的对应Tags */
	UPROPERTY(EditDefaultsOnly, Category = Cooldown)
	FGameplayTagContainer CooldownTags;

	/** 技能是否已经执行了消耗 */
	UPROPERTY()
	bool bCostCommitted = false;

	/** 技能是否已经执行了冷却 */
	UPROPERTY()
	bool bCooldownCommitted = false;

#pragma endregion


#pragma region Requirement // 技能激活条件

protected:
	/** 是否可激活技能 */
	UFUNCTION()
	virtual bool CanActivateAbility() const;

	/** 蓝图端的是否可激活技能 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "Can Activate Ability"))
	bool ReceiveCanActivateAbility() const;

	/** 技能被消息事件激活 */
	UFUNCTION()
	virtual void ActivateAbilityByMessage(const FFireflyMessageEventData EventData);

	/** 蓝图端的技能被消息事件激活 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "Activate Ability By Message"))
	void ReceiveActivateAbilityByMessage(const FFireflyMessageEventData EventData);

	bool bHasBlueprintCanActivate;

	/** 当技能激活或结束时，执行对Owner的Tag更新管理 */
	UFUNCTION()
	void ExecuteAbilityTagRequirementToOwner(bool bIsActivated);

protected:
	/** 该技能的激活需要的正在执行的技能的类型，数组中有一个技能正在激活，都可以让该技能激活 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|AbilityRequired")
	TArray<TSubclassOf<UFireflyAbility>> AbilityClassesRequired;

	/** 该技能的激活是否会取消其前置依赖技能的执行 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|AbilityRequired")
	bool bCancelRequiredAbilities;

	/** 为该做特殊技能资产标记的Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsForAbilityAsset;

	/** 该技能的激活执行会取消带有这些资产标记Tags的技能的执行 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsOfAbilitiesWillBeCanceled;

	/** 该技能的激活执行会阻断带有这些资产标记Tags的技能的执行 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsOfAbilitiesWillBeBlocked;

	/** 该技能激活执行会为Owner添加如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsApplyToOwnerOnActivated;

	/** 该技能激活执行需要技能管理器含有如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsRequireOwnerHasForActivation;

	/** 该技能激活执行期望技能管理器不含如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsBlockActivationOnOwnerHas;

	/** 可以激活该技能的消息事件 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|MessageTrigger")
	FGameplayTagContainer TagsTriggersActivation;

#pragma endregion


#pragma region InputBinding // 输入绑定

protected:
	/** 输入事件：开始 */
	UFUNCTION()
	void OnAbilityInputStarted();

	/** 输入事件内部执行：开始*/
	UFUNCTION()
	virtual void OnAbilityInputStartedInternal();

	/** 本地客户端通知服务端执行输入事件：开始 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputStarted();

	/** 蓝图输入事件：开始 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Started"))
	void ReceiveOnAbilityInputStarted();

	/** 输入事件：执行中 */
	UFUNCTION()
	void OnAbilityInputOngoing();

	/** 输入事件内部执行：执行中*/
	UFUNCTION()
	virtual void OnAbilityInputOngoingInternal();

	/** 本地客户端通知服务端执行输入事件：执行中 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputOngoing();

	/** 蓝图输入事件：执行中 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Ongoing"))
	void ReceiveOnAbilityInputOngoing();

	/** 输入事件：取消 */
	UFUNCTION()
	void OnAbilityInputCanceled();

	/** 输入事件内部执行：取消*/
	UFUNCTION()
	virtual void OnAbilityInputCanceledInternal();

	/** 本地客户端通知服务端执行输入事件：取消 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputCanceled();

	/** 蓝图输入事件：取消 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Canceled"))
	void ReceiveOnAbilityInputCanceled();

	/** 输入事件：触发 */
	UFUNCTION()
	void OnAbilityInputTriggered();

	/** 输入事件内部执行：触发 */
	UFUNCTION()
	virtual void OnAbilityInputTriggeredInternal();

	/** 本地客户端通知服务端执行输入事件：触发 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputTriggered();

	/** 蓝图输入事件：触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Triggered"))
	void ReceiveOnAbilityInputTriggered();

	/** 输入事件：完成 */
	UFUNCTION()
	void OnAbilityInputCompleted();

	/** 输入事件内部执行：完成 */
	UFUNCTION()
	virtual void OnAbilityInputCompletedInternal();

	/** 本地客户端通知服务端执行输入事件：完成 */
	UFUNCTION(Server, Reliable)
	void Server_OnAbilityInputCompleted();

	/** 蓝图输入事件：完成 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Ability Input Completed"))
	void ReceiveOnAbilityInputCompleted();

protected:
	/** 是否在输入事件Triggered时激活技能，如为false，则默认在输入事件Started时激活技能 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|InputRequired")
	bool bActivateOnTriggered = false;

#pragma endregion


#pragma region MontagePlay // 蒙太奇播放

protected:
	/** 获取拥有者的动画实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = true))
	UAnimInstance* GetAnimInstanceOfOwner() const;

	/** 让拥有者播放指定的蒙太奇，内部执行 */
	UFUNCTION()
	void PlayMontageForOwnerInternal(UAnimMontage* MontageToPlay, float PlayRate, FName Section);

	/** 让拥有者播放指定的蒙太奇 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = true))
	float PlayMontageForOwner(UAnimMontage* MontageToPlay, float PlayRate, FName Section, bool bStopOnAbilityEnded);

	/** 本地通知服务端让拥有者播放指定的蒙太奇 */
	UFUNCTION(Server, Reliable)
	void Server_PlayMontageForOwner(UAnimMontage* MontageToPlay, float PlayRate, FName Section, bool bStopOnAbilityEnded);

	/** 服务端广播通知让拥有者播放指定的蒙太奇 */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayMontageForOwner(UAnimMontage* MontageToPlay, float PlayRate, FName Section);

	/** 拥者者播放的蒙太奇结束的事件 */
	UFUNCTION()
	void OnOwnerMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 蓝图端实现的拥者者播放的蒙太奇结束的事件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Owner Montage Ended"))
	void ReceiveOnOwnerMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 拥者者播放的蒙太奇混合出的事件 */
	UFUNCTION()
	void OnOwnerMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	/** 蓝图端实现的拥者者播放的蒙太奇混合出的事件 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "On Owner Montage Blend Out"))
	void ReceiveOnOwnerMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	/** 广播强制停止某个蒙太奇的播放 */
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StopMontagePlaying(UAnimMontage* MontageToStop);

protected:
	FOnMontageEnded OnMontageEnded;
	FOnMontageBlendingOutStarted OnMontageBlendOut;

	UPROPERTY()
	UAnimMontage* MontageToStopOnAbilityEnded;

#pragma endregion
};

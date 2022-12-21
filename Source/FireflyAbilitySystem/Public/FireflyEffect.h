// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbilitySystemTypes.h"
#include "UObject/NoExportTypes.h"
#include "FireflyEffect.generated.h"

class UFireflyAbilitySystemComponent;

/** 效果 */
UCLASS( Blueprintable )
class FIREFLYABILITYSYSTEM_API UFireflyEffect : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Override // 基类重载

public:
	virtual UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;

	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

#pragma endregion


#pragma region Basic // 基础

protected:
	/** 获取效果所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取效果所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyAbilitySystemComponent* GetOwnerManager() const;

	/** 根据效果的动态构造器设置效果属性 */
	UFUNCTION()
	FORCEINLINE void SetupEffectByDynamicConstructor(FFireflyEffectDynamicConstructor EffectSetup);

protected:
	friend UFireflyAbilitySystemComponent;

	/** 效果的唯一标识ID */
	UPROPERTY()
	FName EffectID;

#pragma endregion


#pragma region Duration // 持续时间

protected:
	/** 设置正在执行的效果的剩余持续时间 */
	UFUNCTION()
	void SetTimeRemainingOfDuration(float NewDuration);

	/** 获取正在执行的效果的剩余持续时间 */
	UFUNCTION()
	float GetTimeRemainingOfDuration() const;

	/** 尝试执行或刷新持续时间 */
	UFUNCTION()
	void TryExecuteOrRefreshDuration();

protected:
	/** 效果的持续性策略 */
	UPROPERTY(EditDefaultsOnly, Category = Duration)
	EFireflyEffectDurationPolicy DurationPolicy;

	/** 效果的持续时间，仅在持续策略为“HasDuration”时起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Duration, Meta = (EditCondition = "DurationPolicy == EFireflyEffectDurationPolicy::HasDuration"))
	float Duration;

	/** 效果的持续时间计时器句柄 */
	UPROPERTY()
	FTimerHandle DurationTimer;

#pragma endregion


#pragma region Periodicity // 周期性跳动执行

protected:
	/** 尝试执行或重置周期性逻辑 */
	UFUNCTION()
	void TryExecuteOrResetPeriodicity();
	
protected:
	/** 效果在生效时是否按周期执行逻辑 */
	UPROPERTY(EditDefaultsOnly, Category = Periodicity)
	bool bIsEffectExecutionPeriodic;

	/** 效果的周期间隔时间，尽在周期性策略为“true”时起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Periodicity, Meta = (EditCondition = "bIsEffectExecutionPeriodic == true"))
	float PeriodicInterval;

	/** 效果的周期时间计时器句柄 */
	UPROPERTY()
	FTimerHandle PeriodicityTimer;

#pragma endregion


#pragma region Stacking // 堆叠管理

protected:
	/** 增加该效果的堆叠数 */
	UFUNCTION()
	virtual void AddEffectStack(int32 StackCountToAdd);

	/** 蓝图端实现的增加该效果的堆叠数 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "AddEffectStack"))
	void ReceiveAddEffectStack(int32 StackCountToAdd);

	/** 减少该效果的堆叠数 */
	UFUNCTION()
	virtual bool ReduceEffectStack(int32 StackCountToReduce);

	/** 蓝图端实现的减少该效果的堆叠数 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "ReduceEffectStack"))
	void ReceiveReduceEffectStack(int32 StackCountToReduce);

	/** 尝试执行效果的堆叠数达到上限时的逻辑 */
	UFUNCTION()
	virtual bool TryExecuteEffectStackOverflow();

	/** 蓝图端实现的当该效果的堆叠数达到上限时触发的逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "ReduceEffectStack"))
	void ReceiveExecuteEffectStackOverflow();

protected:
	/** 该效果选择的堆叠策略 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	EFireflyEffectStackingPolicy StackingPolicy;

	/** 效果的堆叠量上限，仅在堆叠策略为“StackHasLimit”时起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking, Meta = (EditCondition = "StackingPolicy == EFireflyEffectStackingPolicy::StackHasLimit"))
	int32 StackingLimitation;

	/** 效果有新的实例被执行或堆叠数量增加时，是否刷新持续时间 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	bool bShouldRefreshDurationOnStacking;

	/** 效果有新的实例被执行或堆叠数量增加时，是否重置周期性 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	bool bShouldResetPeriodicityOnStacking;

	/** 效果的堆叠数达到上限时，触发的额外的效果 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	TArray<TSubclassOf<UFireflyEffect>> OverflowEffects;

	/** 效果的堆叠数达到上限时，是否拒绝新的堆叠应用 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	bool bDenyNewStackingOnOverflow;

	/** 效果的堆叠数达到上限时，是否清除所有的堆叠数 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	bool bClearStackingOnOverflow;

	/** 堆叠到期策略，即效果的持续时间的到期对堆叠的影响 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	EFireflyEffectDurationPolicyOnStackingExpired StackingExpirationPolicy;

	/** 效果的堆叠数 */
	UPROPERTY()
	int32 StackCount;

#pragma endregion


#pragma region Modifiers // 属性修改器

protected:
	/** 该效果携带的属性修改器 */
	UPROPERTY(EditDefaultsOnly, Category = Modifier)
	TArray<FFireflyEffectModifierData> Modifiers;

	/** 该效果携带的特殊属性 */
	UPROPERTY(EditDefaultsOnly, Category = Modifier)
	TArray<FFireflySpecificProperty> SpecificProperties;

#pragma endregion


#pragma region Application // 应用管理

protected:
	/** 效果被应用时的逻辑 */
	UFUNCTION()
	virtual void ApplyEffect(AActor* InInstigator = nullptr, AActor* InTarget = nullptr, int32 StackToApply = 1);

	/** 蓝图端实现的效果被应用时的逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "Apply Effect"))
	void ReceiveApplyEffect();

	/** 不为Instant的效果第一次被应用时执行的逻辑 */
	UFUNCTION()
	virtual void ApplyEffectFirstTime(UFireflyAbilitySystemComponent* Manager);

	/** 效果的执行逻辑 */
	UFUNCTION()
	virtual void ExecuteEffect();

	/** 蓝图端实现的效果的执行逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "Execute Effect"))
	void ReceiveExecuteEffect();

	/** 效果持续时间到期时执行的逻辑 */
	UFUNCTION()
	virtual void ExecuteEffectExpiration();

	/** 蓝图端实现的效果持续时间到期时执行的逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "Execute Effect Expiration"))
	void ReceiveExecuteEffectExpiration();

	/** 效果被移除时的逻辑 */
	UFUNCTION()
	virtual void RemoveEffect();

	/** 蓝图端实现的效果被移除时的逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "Remove Effect"))
	void ReceiveRemoveEffect();

public:
	/** 获取效果的所有发起者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	FORCEINLINE TArray<AActor*> GetInstigators() const { return Instigators; }

	/** 获取效果的接收者（应该和OwnerActor相同） */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	FORCEINLINE AActor* GetTarget() const { return Target; }

protected:
	/** 该效果携带的特殊属性 */
	UPROPERTY(EditDefaultsOnly, Category = Instancing)
	EFireflyEffectInstigatorApplicationPolicy InstigatorApplicationPolicy;

	/** 效果执行的发起者 */
	UPROPERTY()
	TArray<AActor*> Instigators;

	/** 效果执行的接受者 */
	UPROPERTY()
	AActor* Target;

#pragma endregion


#pragma region TagRequirement // 应用条件

protected:
	/** 当拥有者的管理器的TagContainer更新时触发的代理 */
	UFUNCTION()
	void OnOwnerTagContainerUpdated(FGameplayTagContainer OwnerTagContainer);

	/** 切换效果运行时的生效和暂时无效 */
	UFUNCTION()
	void SwitchEffectOngoingValidation(bool bIsEffective);

	/** 当效果被应用或移除时，执行对Owner的Tag更新管理 */
	UFUNCTION()
	void ExecuteEffectTagRequirementToOwner(bool bIsApplied);

protected:
	/** 效果的标签Tags，仅用于描述修饰效果资产 */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsForEffectAsset;

	/** 效果被应用时，会应用给管理器组件的Tags */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsApplyToOwnerOnApplied;

	/** 效果被应用时，管理器组件需要拥有这些Tags，该效果才能真的起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsRequiredOngoing;

	/** 效果被应用时，管理器组件需要没有这些Tags，该效果才能真的起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsBlockedOngoing;

	/** 该效果的激活应用会取消带有这些资产标记Tags的效果的应用 */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsOfEffectsWillBeRemoved;

	/** 该效果的激活应用会阻断带有这些资产标记Tags的效果的应用 */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsOfEffectsWillBeBlocked;

	/** 该效果的激活应用需要管理器含有如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsRequireOwnerHasForApplication;

	/** 该效果的激活应用期望管理器不含如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer TagsBlockApplicationOnOwnerHas;

	/** 该效果在运行时是否处于生效状态 */
	UPROPERTY()
	bool bOngoingEffective = true;

#pragma endregion	
};

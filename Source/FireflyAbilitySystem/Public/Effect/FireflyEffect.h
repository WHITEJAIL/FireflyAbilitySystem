// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbilitySystemTypes.h"
#include "UObject/NoExportTypes.h"
#include "FireflyEffect.generated.h"

class UFireflyEffectManagerComponent;

/** 效果 */
UCLASS( Blueprintable )
class FIREFLYABILITYSYSTEM_API UFireflyEffect : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Basic

public:
	virtual UWorld* GetWorld() const override;

protected:
	/** 获取效果所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取效果所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyEffectManagerComponent* GetOwnerManager() const;

protected:
	friend class UFireflyEffectManagerComponent;

#pragma endregion


#pragma region Duration

protected:
	/** 设置正在执行的效果的剩余持续时间 */
	UFUNCTION()
	void SetTimeRemainingOfDuration(float NewDuration);

	/** 获取正在执行的效果的剩余持续时间 */
	UFUNCTION()
	float GetTimeRemainingOfDuration() const;

	/** 当有新实例被执行或对叠数增加时，尝试刷新持续时间 */
	UFUNCTION()
	void TryRefreshDurationOnStacking();

	/** 执行持续时间归零时，效果的堆叠到期策略 */
	UFUNCTION()
	void ExecuteStackingExpirationPolicy();

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


#pragma region Periodicity

protected:
	/** 当有新实例被执行或对叠数增加时，尝试重置周期性 */
	UFUNCTION()
	void TryResetPeriodicityOnStacking();
	
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


#pragma region Stacking

protected:
	/** 增加该效果的堆叠数 */
	UFUNCTION()
	virtual void AddEffectStack(int32 StackCountToAdd);

	/** 蓝图端实现的增加该效果的堆叠数 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "AddEffectStack"))
	void ReceiveAddEffectStack(int32 StackCountToAdd);

	/** 减少该效果的堆叠数 */
	UFUNCTION()
	virtual void ReduceEffectStack(int32 StackCountToReduce);

	/** 蓝图端实现的减少该效果的堆叠数 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "ReduceEffectStack"))
	void ReceiveReduceEffectStack(int32 StackCountToReduce);

	/** 当该效果的堆叠数达到上限时触发的逻辑 */
	UFUNCTION()
	virtual void ExecuteEffectStackOverflow();

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

	/** 效果的持续时间到期时，对堆叠的影响 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	EFireflyEffectDurationPolicyOnStackingExpired StackingExpirationPolicy;

	/** 效果的堆叠数 */
	UPROPERTY()
	int32 StackCount;

#pragma endregion


#pragma region Modifiers

protected:
	/** 该效果携带的属性修改器 */
	UPROPERTY(EditDefaultsOnly, Category = Modifier)
	TArray<FFireflyEffectModifierData> Modifiers;

	/** 该效果携带的特殊属性 */
	UPROPERTY(EditDefaultsOnly, Category = Modifier)
	TArray<FFireflySpecificProperty> SpecificProperties;

#pragma endregion


#pragma region Application

protected:
	/** 效果被应用时的逻辑 */
	UFUNCTION()
	virtual void ApplyEffect(AActor* InInstigator = nullptr, AActor* InTarget = nullptr, int32 StackToApply = 1);

	/** 蓝图端实现的效果被应用时的逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "Apply Effect"))
	void ReceiveApplyEffect();	

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

protected:
	/** 效果执行的发起者 */
	UPROPERTY(BlueprintReadOnly, Category = "FireflyAbilitySystem|Effect")
	TArray<AActor*> Instigators;

	/** 效果执行的接受者 */
	UPROPERTY(BlueprintReadOnly, Category = "FireflyAbilitySystem|Effect")
	AActor* Target;

	/** 该效果携带的特殊属性 */
	UPROPERTY(BlueprintReadOnly, Category = "FireflyAbilitySystem|Effect")
	EFireflyEffectInstigatorApplicationPolicy InstigatorApplicationPolicy;

#pragma endregion
	
};

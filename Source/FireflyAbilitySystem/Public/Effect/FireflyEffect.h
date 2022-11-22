// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireflyAbilitySystemTypes.h"
#include "UObject/NoExportTypes.h"
#include "FireflyEffect.generated.h"

class UFireflyEffectManagerComponent;

/** 效果 */
UCLASS(Blueprintable)
class FIREFLYABILITYSYSTEM_API UFireflyEffect : public UObject
{
	GENERATED_BODY()

#pragma region Basic

public:
	virtual UWorld* GetWorld() const override;

protected:
	/** 获取技能所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取技能所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyEffectManagerComponent* GetOwnerManager() const;

protected:
	friend class UFireflyEffectManagerComponent;

#pragma endregion


#pragma region Duration

public:
	/** 设置正在执行的效果的剩余持续时间 */
	UFUNCTION()
	void SetTimeRemainingOfDuration(float NewDuration);

	/** 获取正在执行的效果的剩余持续时间 */
	UFUNCTION()
	float GetTimeRemainingOfDuration() const;

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
	void AddEffectStack(int32 StackCountToAdd);

	/** 增加该效果的堆叠数 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "AddEffectStack"))
	void ReceiveAddEffectStack(int32 StackCountToAdd);

	/** 增加该效果的堆叠数 */
	UFUNCTION()
	void ReduceEffectStack(int32 StackCountToReduce);

	/** 增加该效果的堆叠数 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Effect", Meta = (DisplayName = "ReduceEffectStack"))
	void ReceiveReduceEffectStack(int32 StackCountToReduce);

protected:
	/** 该效果选择的堆叠策略 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	EFireflyEffectStackingPolicy StackingPolicy;

	/** 效果的持续时间，仅在持续策略为“HasDuration”时起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking, Meta = (EditCondition = "StackingPolicy == EFireflyEffectStackingPolicy::StackHasLimit"))
	uint32 StackLimitation;

	/** 效果有新的实例被执行或堆叠数量增加时，是否刷新持续时间 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	bool bShouldRefreshDurationOnStacking;

	/** 效果有新的实例被执行或堆叠数量增加时，是否重置周期性 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	bool bShouldResetPeriodicityOnStacking;

	/** 效果的堆叠到期时，对持续时间的影响 */
	UPROPERTY(EditDefaultsOnly, Category = Stacking)
	EFireflyEffectDurationPolicyOnStackingExpired StackExpirationPolicy;

	/** 效果的堆叠数 */
	UPROPERTY()
	int32 StackCount;

#pragma endregion


#pragma region Modifiers

protected:
	/** 该效果携带的属性修改器 */
	UPROPERTY(EditDefaultsOnly, Category = Modifier)
	TArray<FFireflyEffectModifier> Modifiers;

	/** 该效果携带的特殊属性 */
	UPROPERTY(EditDefaultsOnly, Category = Modifier)
	TArray<FFireflySpecificProperty> SpecificProperties;

#pragma endregion


#pragma region Execution

protected:
	/** 效果执行的发起者 */
	UPROPERTY(BlueprintReadOnly, Category = "FireflyAbilitySystem|Effect")
	AActor* Instigator;

	/** 效果执行的接受者 */
	UPROPERTY(BlueprintReadOnly, Category = "FireflyAbilitySystem|Effect")
	AActor* Target;

#pragma endregion
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FireflyEffect.generated.h"

class UFireflyEffectManagerComponent;

/** 效果的持续性策略 */
UENUM()
enum class EFireflyEffectDurationPolicy : uint8
{
	/** 该效果会在执行生效后立即结束 */
	Instant,
	/** 该效果会永久执行生效，除非被手动中止 */
	Infinite,
	/** 该效果会按照设定的持续时间执行生效 */
	HasDuration
};

/** 效果的堆叠性策略 */
UENUM()
enum class EFireflyEffectStackingPolicy : uint8
{
	/** 该效果不会堆叠 */
	None,
	/** 该效果会堆叠，且堆叠量无限制 */
	StackNoLimit,
	/** 该效果会堆叠，但堆叠量有限制 */
	StackHasLimit
};

/** 效果的堆叠到期时，选择的持续时间策略 */
UENUM()
enum class EFireflyEffectDurationPolicyOnStackingExpired : uint8
{
	/** 当效果到期时，清理效果的所有堆叠  */
	ClearEntireStack,

	/** 当前堆栈计数将递减1，并刷新持续时间。效果并没有被“重新执行”，只是继续以一个更少的堆栈存在。 */
	RemoveSingleStackAndRefreshDuration,

	/** 游戏效果的持续时间被刷新。这本质上使效果在持续时间上无限。这可用于通过OnStackCountChange回调手动处理堆栈递减 */
	RefreshDuration
};

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
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取技能所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyEffectManagerComponent* GetOwnerManager() const;

	friend class UFireflyEffectManagerComponent;

#pragma endregion


#pragma region Duration

protected:
	/** 效果的持续性策略 */
	UPROPERTY(EditDefaultsOnly, Category = Duration)
	EFireflyEffectDurationPolicy DurationPolicy;

	/** 效果的持续时间，仅在持续策略为“HasDuration”时起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Duration, Meta = (EditCondition = "DurationPolicy == EFireflyEffectDurationPolicy::HasDuration"))
	float Duration;

#pragma endregion


#pragma region Periodicity

protected:
	/** 效果在生效时是否按周期执行逻辑 */
	UPROPERTY(EditDefaultsOnly, Category = Periodicity)
	bool bIsEffectExecutionPeriodic;

	/** 效果的周期间隔时间，尽在周期性策略为“true”时起作用 */
	UPROPERTY(EditDefaultsOnly, Category = Periodicity, Meta = (EditCondition = "bIsEffectExecutionPeriodic == true"))
	float PeriodicInterval;

#pragma endregion


#pragma region Stacking

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

#pragma endregion


#pragma region Modifiers

#pragma endregion
	
};

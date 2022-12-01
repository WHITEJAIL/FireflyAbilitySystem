// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "FireflyAbilitySystemTypes.generated.h"

class UFireflyAttribute;
class UFireflyEffect;

#pragma region Attribute

/** 个人技能系统的属性枚举，在项目设置中定义DisplayName */
UENUM(BlueprintType)
enum EFireflyAttributeType
{
	AttributeType_Default UMETA(DisplayName = "Default"),
	AttributeType01 UMETA(Hidden),
	AttributeType02 UMETA(Hidden),
	AttributeType03 UMETA(Hidden),
	AttributeType04 UMETA(Hidden),
	AttributeType05 UMETA(Hidden),
	AttributeType06 UMETA(Hidden),
	AttributeType07 UMETA(Hidden),
	AttributeType08 UMETA(Hidden),
	AttributeType09 UMETA(Hidden),
	AttributeType10 UMETA(Hidden),
	AttributeType11 UMETA(Hidden),
	AttributeType12 UMETA(Hidden),
	AttributeType13 UMETA(Hidden),
	AttributeType14 UMETA(Hidden),
	AttributeType15 UMETA(Hidden),
	AttributeType16 UMETA(Hidden),
	AttributeType17 UMETA(Hidden),
	AttributeType18 UMETA(Hidden),
	AttributeType19 UMETA(Hidden),
	AttributeType20 UMETA(Hidden),
	AttributeType21 UMETA(Hidden),
	AttributeType22 UMETA(Hidden),
	AttributeType23 UMETA(Hidden),
	AttributeType24 UMETA(Hidden),
	AttributeType25 UMETA(Hidden),
	AttributeType26 UMETA(Hidden),
	AttributeType27 UMETA(Hidden),
	AttributeType28 UMETA(Hidden),
	AttributeType29 UMETA(Hidden),
	AttributeType30 UMETA(Hidden),
	AttributeType31 UMETA(Hidden),
	AttributeType32 UMETA(Hidden),
	AttributeType33 UMETA(Hidden),
	AttributeType34 UMETA(Hidden),
	AttributeType35 UMETA(Hidden),
	AttributeType36 UMETA(Hidden),
	AttributeType37 UMETA(Hidden),
	AttributeType38 UMETA(Hidden),
	AttributeType39 UMETA(Hidden),
	AttributeType40 UMETA(Hidden),
	AttributeType41 UMETA(Hidden),
	AttributeType42 UMETA(Hidden),
	AttributeType43 UMETA(Hidden),
	AttributeType44 UMETA(Hidden),
	AttributeType45 UMETA(Hidden),
	AttributeType46 UMETA(Hidden),
	AttributeType47 UMETA(Hidden),
	AttributeType48 UMETA(Hidden),
	AttributeType49 UMETA(Hidden),
	AttributeType50 UMETA(Hidden),
	AttributeType51 UMETA(Hidden),
	AttributeType52 UMETA(Hidden),
	AttributeType53 UMETA(Hidden),
	AttributeType54 UMETA(Hidden),
	AttributeType55 UMETA(Hidden),
	AttributeType56 UMETA(Hidden),
	AttributeType57 UMETA(Hidden),
	AttributeType58 UMETA(Hidden),
	AttributeType59 UMETA(Hidden),
	AttributeType60 UMETA(Hidden),
	AttributeType61 UMETA(Hidden),
	AttributeType62 UMETA(Hidden),
	AttributeType_Max UMETA(Hidden)
};

/** 属性的修改器操作符 */
UENUM()
enum class EFireflyAttributeModOperator : uint8
{
	/** 无操作 */
	None,

	/** 加法操作 */
	Plus,

	/** 减法操作 */
	Minus,

	/** 乘法操作 */
	Multiply,

	/** 除法操作 */
	Divide,

	/** 内部覆盖操作 */
	InnerOverride,

	/** 全局覆盖操作 */
	OuterOverride
};

/** 属性的构造器 */
USTRUCT(BlueprintType)
struct FFireflyAttributeConstructor
{
	GENERATED_BODY()

public:
	/** 属性的类型 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EFireflyAttributeType> AttributeType = AttributeType_Default;

	/** 属性的初始值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AttributeInitValue = 0.f;

	/** 属性是否拥有夹值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bAttributeHasRange = false;

	/** 属性的范围最小值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RangeMinValue = 0.f;

	/** 属性的范围最大值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RangeMaxValue = 0.f;

	/** 属性的范围最大值属性类型，基于另一个属性的当前值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EFireflyAttributeType> RangeMaxValueType = AttributeType_Default;
};

#pragma endregion


#pragma region Effect

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

/** 效果的作用和发起者之间关系的选择策略 */
UENUM()
enum class EFireflyEffectInstigatorApplicationPolicy : uint8
{
	/** 同一个效果管理器中，只允许一个发起者应用的该效果实例存在 */
	OnlyOneInstigator,

	/** 同一个效果管理器中，不同的发起者共享同一个该效果实例 */
	InstigatorsShareOne,

	/** 同一个效果管理器中，不同的发起者生成不同的该效果实例 */
	InstigatorsApplyTheirOwn
};

/** 效果携带的属性修改器 */
USTRUCT(BlueprintType)
struct FFireflyEffectModifierData
{
	GENERATED_BODY()

public:
	/** 修改的属性类型 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EFireflyAttributeType> AttributeType = EFireflyAttributeType::AttributeType_Default;

	/** 修改的操作符 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EFireflyAttributeModOperator ModOperator = EFireflyAttributeModOperator::None;

	/** 修改的操作值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ModValue = 0.f;

	FFireflyEffectModifierData() {}

	FORCEINLINE bool operator==(const FFireflyEffectModifierData& Other) const
	{
		return AttributeType == Other.AttributeType && ModOperator == Other.ModOperator && ModValue == Other.ModValue;
	}

	FORCEINLINE bool TypeEqual(const FFireflyEffectModifierData& Other) const
	{
		return AttributeType == Other.AttributeType && ModOperator == Other.ModOperator;
	}
};

/** 效果携带的特殊属性 */
USTRUCT(BlueprintType)
struct FFireflySpecificProperty
{
	GENERATED_BODY()

public:
	/** 表示特殊属性的标签 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FGameplayTag PropertyTag = FGameplayTag::EmptyTag;

	/** 表示特殊属性的标签 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float PropertyValue = 0.f;

	FFireflySpecificProperty() {}

	FORCEINLINE bool operator==(const FFireflySpecificProperty& Other) const
	{
		return PropertyTag.MatchesTagExact(Other.PropertyTag) && PropertyValue == Other.PropertyValue;
	}
};

/** 基于某个效果的类型动态构建效果的桥接数据 */
USTRUCT(BlueprintType)
struct FFireflyEffectDynamicHandle
{
	GENERATED_BODY()

public:
	/** 构建效果基于的类 */
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UFireflyEffect> EffectType = nullptr;

	/** 构建效果要分配的属性修改器，需要原始的属性类中定义了下述的属性修改器 */
	UPROPERTY(BlueprintReadWrite)
	TArray<FFireflyEffectModifierData> AssignableEffectModifiers = TArray<FFireflyEffectModifierData>{};

	/** 构建效果要添加的特殊属性 */
	UPROPERTY(BlueprintReadWrite)
	TArray<FFireflySpecificProperty> SpecificPropertiesToAdd = TArray<FFireflySpecificProperty>{};

	FFireflyEffectDynamicHandle() {}
};

/** 效果的动态构建器 */
USTRUCT(BlueprintType)
struct FFireflyEffectDynamicConstructor
{
	GENERATED_BODY()

public:
	/** 效果的持续性策略 */
	UPROPERTY(BlueprintReadWrite, Category = Duration)
	EFireflyEffectDurationPolicy DurationPolicy = EFireflyEffectDurationPolicy::Instant;

	/** 效果的持续时间，仅在持续策略为“HasDuration”时起作用 */
	UPROPERTY(BlueprintReadWrite, Category = Duration, Meta = (EditCondition = "DurationPolicy == EFireflyEffectDurationPolicy::HasDuration"))
	float Duration = 0.f;

	/** 效果在生效时是否按周期执行逻辑 */
	UPROPERTY(BlueprintReadWrite, Category = Periodicity)
	bool bIsEffectExecutionPeriodic = false;

	/** 效果的周期间隔时间，尽在周期性策略为“true”时起作用 */
	UPROPERTY(BlueprintReadWrite, Category = Periodicity, Meta = (EditCondition = "bIsEffectExecutionPeriodic == true"))
	float PeriodicInterval = 0.f;

	/** 该效果选择的堆叠策略 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking)
	EFireflyEffectStackingPolicy StackingPolicy = EFireflyEffectStackingPolicy::None;

	/** 效果的持续时间，仅在持续策略为“HasDuration”时起作用 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking, Meta = (EditCondition = "StackingPolicy == EFireflyEffectStackingPolicy::StackHasLimit"))
	int32 StackingLimitation = 0;

	/** 效果有新的实例被执行或堆叠数量增加时，是否刷新持续时间 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking)
	bool bShouldRefreshDurationOnStacking = false;

	/** 效果有新的实例被执行或堆叠数量增加时，是否重置周期性 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking)
	bool bShouldResetPeriodicityOnStacking = false;

	/** 效果的堆叠数达到上限时，触发的额外的效果 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking)
	TArray<TSubclassOf<UFireflyEffect>> OverflowEffects = TArray<TSubclassOf<UFireflyEffect>>{};

	/** 效果的堆叠数达到上限时，是否拒绝新的堆叠应用 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking)
	bool bDenyNewStackingOnOverflow = false;

	/** 效果的堆叠数达到上限时，是否清除所有的堆叠数 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking)
	bool bClearStackingOnOverflow = false;

	/** 效果的堆叠到期时，对持续时间的影响 */
	UPROPERTY(BlueprintReadWrite, Category = Stacking)
	EFireflyEffectDurationPolicyOnStackingExpired StackingExpirationPolicy = EFireflyEffectDurationPolicyOnStackingExpired::ClearEntireStack;

	/** 该效果携带的属性修改器 */
	UPROPERTY(BlueprintReadWrite, Category = Modifier)
	TArray<FFireflyEffectModifierData> Modifiers = TArray<FFireflyEffectModifierData>{};

	/** 该效果携带的特殊属性 */
	UPROPERTY(BlueprintReadWrite, Category = Modifier)
	TArray<FFireflySpecificProperty> SpecificProperties = TArray<FFireflySpecificProperty>{};

	/** 该效果携带的特殊属性 */
	UPROPERTY(BlueprintReadWrite, Category = Application)
	EFireflyEffectInstigatorApplicationPolicy InstigatorApplicationPolicy = EFireflyEffectInstigatorApplicationPolicy::InstigatorsApplyTheirOwn;

	/** 效果的标签Tags，仅用于描述修饰效果资产 */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsForEffectAsset = FGameplayTagContainer::EmptyContainer;

	/** 效果被应用时，会应用给管理器组件的Tags */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsApplyToOwnerOnApplied = FGameplayTagContainer::EmptyContainer;

	/** 效果被应用时，管理器组件需要拥有这些Tags，该效果才能真的起作用 */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsRequiredOngoing = FGameplayTagContainer::EmptyContainer;

	/** 效果被应用时，管理器组件需要没有这些Tags，该效果才能真的起作用 */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsBlockedOngoing = FGameplayTagContainer::EmptyContainer;

	/** 该效果的激活应用会取消带有这些资产标记Tags的效果的应用 */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsOfEffectsWillBeRemoved = FGameplayTagContainer::EmptyContainer;

	/** 该效果的激活应用会阻断带有这些资产标记Tags的效果的应用 */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsOfEffectsWillBeBlocked = FGameplayTagContainer::EmptyContainer;

	/** 该效果的激活应用需要管理器含有如下Tags */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsRequireOwnerHasForApplication = FGameplayTagContainer::EmptyContainer;

	/** 该效果的激活应用期望管理器不含如下Tags */
	UPROPERTY(BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer TagsBlockApplicationOnOwnerHas = FGameplayTagContainer::EmptyContainer;
	
};

#pragma endregion


#pragma region Ability

#pragma endregion
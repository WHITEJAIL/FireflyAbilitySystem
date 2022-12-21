// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "FireflyAbilitySystemTypes.generated.h"

class UFireflyAbility;
class UFireflyAttribute;
class UFireflyEffect;
class UFireflyEffectModifierCalculator;

#pragma region Attribute // 属性

/** 个人技能系统的属性枚举，在项目设置中定义DisplayName */
UENUM(BlueprintType)
enum EFireflyAttributeType
{
	AttributeType_Default UMETA(DisplayName = "Default"),
	AttributeType001 UMETA(Hidden),
	AttributeType002 UMETA(Hidden),
	AttributeType003 UMETA(Hidden),
	AttributeType004 UMETA(Hidden),
	AttributeType005 UMETA(Hidden),
	AttributeType006 UMETA(Hidden),
	AttributeType007 UMETA(Hidden),
	AttributeType008 UMETA(Hidden),
	AttributeType009 UMETA(Hidden),
	AttributeType010 UMETA(Hidden),
	AttributeType011 UMETA(Hidden),
	AttributeType012 UMETA(Hidden),
	AttributeType013 UMETA(Hidden),
	AttributeType014 UMETA(Hidden),
	AttributeType015 UMETA(Hidden),
	AttributeType016 UMETA(Hidden),
	AttributeType017 UMETA(Hidden),
	AttributeType018 UMETA(Hidden),
	AttributeType019 UMETA(Hidden),
	AttributeType020 UMETA(Hidden),
	AttributeType021 UMETA(Hidden),
	AttributeType022 UMETA(Hidden),
	AttributeType023 UMETA(Hidden),
	AttributeType024 UMETA(Hidden),
	AttributeType025 UMETA(Hidden),
	AttributeType026 UMETA(Hidden),
	AttributeType027 UMETA(Hidden),
	AttributeType028 UMETA(Hidden),
	AttributeType029 UMETA(Hidden),
	AttributeType030 UMETA(Hidden),
	AttributeType031 UMETA(Hidden),
	AttributeType032 UMETA(Hidden),
	AttributeType033 UMETA(Hidden),
	AttributeType034 UMETA(Hidden),
	AttributeType035 UMETA(Hidden),
	AttributeType036 UMETA(Hidden),
	AttributeType037 UMETA(Hidden),
	AttributeType038 UMETA(Hidden),
	AttributeType039 UMETA(Hidden),
	AttributeType040 UMETA(Hidden),
	AttributeType041 UMETA(Hidden),
	AttributeType042 UMETA(Hidden),
	AttributeType043 UMETA(Hidden),
	AttributeType044 UMETA(Hidden),
	AttributeType045 UMETA(Hidden),
	AttributeType046 UMETA(Hidden),
	AttributeType047 UMETA(Hidden),
	AttributeType048 UMETA(Hidden),
	AttributeType049 UMETA(Hidden),
	AttributeType050 UMETA(Hidden),
	AttributeType051 UMETA(Hidden),
	AttributeType052 UMETA(Hidden),
	AttributeType053 UMETA(Hidden),
	AttributeType054 UMETA(Hidden),
	AttributeType055 UMETA(Hidden),
	AttributeType056 UMETA(Hidden),
	AttributeType057 UMETA(Hidden),
	AttributeType058 UMETA(Hidden),
	AttributeType059 UMETA(Hidden),
	AttributeType060 UMETA(Hidden),
	AttributeType061 UMETA(Hidden),
	AttributeType062 UMETA(Hidden),
	AttributeType063 UMETA(Hidden),
	AttributeType064 UMETA(Hidden),
	AttributeType065 UMETA(Hidden),
	AttributeType066 UMETA(Hidden),
	AttributeType067 UMETA(Hidden),
	AttributeType068 UMETA(Hidden),
	AttributeType069 UMETA(Hidden),
	AttributeType070 UMETA(Hidden),
	AttributeType071 UMETA(Hidden),
	AttributeType072 UMETA(Hidden),
	AttributeType073 UMETA(Hidden),
	AttributeType074 UMETA(Hidden),
	AttributeType075 UMETA(Hidden),
	AttributeType076 UMETA(Hidden),
	AttributeType077 UMETA(Hidden),
	AttributeType078 UMETA(Hidden),
	AttributeType079 UMETA(Hidden),
	AttributeType080 UMETA(Hidden),
	AttributeType081 UMETA(Hidden),
	AttributeType082 UMETA(Hidden),
	AttributeType083 UMETA(Hidden),
	AttributeType084 UMETA(Hidden),
	AttributeType085 UMETA(Hidden),
	AttributeType086 UMETA(Hidden),
	AttributeType087 UMETA(Hidden),
	AttributeType088 UMETA(Hidden),
	AttributeType089 UMETA(Hidden),
	AttributeType090 UMETA(Hidden),
	AttributeType091 UMETA(Hidden),
	AttributeType092 UMETA(Hidden),
	AttributeType093 UMETA(Hidden),
	AttributeType094 UMETA(Hidden),
	AttributeType095 UMETA(Hidden),
	AttributeType096 UMETA(Hidden),
	AttributeType097 UMETA(Hidden),
	AttributeType098 UMETA(Hidden),
	AttributeType099 UMETA(Hidden),
	AttributeType100 UMETA(Hidden),
	AttributeType101 UMETA(Hidden),
	AttributeType102 UMETA(Hidden),
	AttributeType103 UMETA(Hidden),
	AttributeType104 UMETA(Hidden),
	AttributeType105 UMETA(Hidden),
	AttributeType106 UMETA(Hidden),
	AttributeType107 UMETA(Hidden),
	AttributeType108 UMETA(Hidden),
	AttributeType109 UMETA(Hidden),
	AttributeType110 UMETA(Hidden),
	AttributeType111 UMETA(Hidden),
	AttributeType112 UMETA(Hidden),
	AttributeType113 UMETA(Hidden),
	AttributeType114 UMETA(Hidden),
	AttributeType115 UMETA(Hidden),
	AttributeType116 UMETA(Hidden),
	AttributeType117 UMETA(Hidden),
	AttributeType118 UMETA(Hidden),
	AttributeType119 UMETA(Hidden),
	AttributeType120 UMETA(Hidden),
	AttributeType121 UMETA(Hidden),
	AttributeType122 UMETA(Hidden),
	AttributeType123 UMETA(Hidden),
	AttributeType124 UMETA(Hidden),
	AttributeType125 UMETA(Hidden),
	AttributeType126 UMETA(Hidden),
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


#pragma region Effect // 效果

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
	/** 修改器针对的属性类型 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EFireflyAttributeType> AttributeType = EFireflyAttributeType::AttributeType_Default;

	/** 修改器执行的操作符 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EFireflyAttributeModOperator ModOperator = EFireflyAttributeModOperator::None;

	/** 修改器的操作值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ModValue = 0.f;

	/** 修改器计算操作值使用的计算器 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<UFireflyEffectModifierCalculator> CalculatorClass;

	/** 修改器起作用时生成的计算器实例 */
	UPROPERTY()
	UFireflyEffectModifierCalculator* CalculatorInstance = nullptr;

	FFireflyEffectModifierData() {}

	FORCEINLINE bool operator==(const FFireflyEffectModifierData& Other) const
	{
		bool bModValueEqual = ModValue == Other.ModValue;
		if (CalculatorInstance != nullptr && Other.CalculatorInstance != nullptr)
		{
			bModValueEqual = CalculatorInstance == Other.CalculatorInstance;
		}
		return AttributeType == Other.AttributeType && ModOperator == Other.ModOperator && bModValueEqual;
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

/** 效果的动态构建器 */
USTRUCT(BlueprintType)
struct FFireflyEffectDynamicConstructor
{
	GENERATED_BODY()

public:
	/** 效果的类型 */
	UPROPERTY(BlueprintReadWrite, Category = Basic)
	TSubclassOf<UFireflyEffect> EffectType = nullptr;

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

	FFireflyEffectDynamicConstructor() {}
};

#pragma endregion


#pragma region Ability // 技能

#pragma endregion


#pragma region DataDriven // 数据驱动

/** 技能的数据表，项目如果期望用数据驱动技能系统运行，要让技能的数据表结构继承 */
USTRUCT(BlueprintType)
struct FFireflyAbilityTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 技能类 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSoftClassPtr<UFireflyAbility> AbilityClass = nullptr;
};

/** 效果的数据表，项目如果期望用数据驱动技能系统运行，要让效果的数据表结构继承 */
USTRUCT(BlueprintType)
struct FFireflyEffectTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 效果类 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSoftClassPtr<UFireflyEffect> EffectClass = nullptr;
};

#pragma endregion


#pragma region MessageEvent // 消息事件

/** 通知事件携带的数据 */
USTRUCT(BlueprintType)
struct FFireflyMessageEventData
{
	GENERATED_BODY()

public:
	/** 触发该事件的标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EventTag = FGameplayTag::EmptyTag;

	/** 该事件的发起者 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Instigator = nullptr;

	/** 该事件的接收者 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Target = nullptr;

	/** 该事件携带的对象实例 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UObject*> OptionalObjects;

	/** 该事件携带的数据信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> EventMagnitudes;

	/** 该事件携带的字符信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EventStrings;
};

#pragma endregion
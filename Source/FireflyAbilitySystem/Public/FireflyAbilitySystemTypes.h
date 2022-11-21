// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "FireflyAbilitySystemTypes.generated.h"

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

/** 效果的属性修改操作符 */
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

/** 效果携带的属性修改器 */
USTRUCT(BlueprintType)
struct FFireflyEffectModifier
{
	GENERATED_BODY()

	/** 修改的属性类型 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EFireflyAttributeType> Attribute = EFireflyAttributeType::AttributeType_Default;

	/** 修改的操作符 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EFireflyAttributeModOperator ModOperator = EFireflyAttributeModOperator::None;

	/** 修改的操作值 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ModValue = 0.f;

	FFireflyEffectModifier() {}

	FORCEINLINE bool operator==(const FFireflyEffectModifier& Other) const
	{
		return Attribute == Other.Attribute && ModOperator == Other.ModOperator && ModValue == Other.ModValue;
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

#pragma endregion
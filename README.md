# 属性模块：Attribute

## 基础概念

### 属性的元素构成

- BaseValue：属性的基础值，只能被 **Instant**效果 或 **周期性跳动执行**效果 的属性修改器修改
- BaseValueToUse：属性用于计算 **CurrentValue** 的基础值，默认为 **BaseValue**
- CurrentValue：属性的当前值，结合属性的当前值和属性的所有修改器 **计算** 得出的值，会在属性的当前值被更改、属性的修改器被添加或移除时 **更新**
- PlusMods：属性的所有 **有持续时间的** 加法修改器，计算公式中会把所有修改器的值相加放入公式
- MinusMods：属性的所有 **有持续时间的** 减法修改器，计算公式中会把所有修改器的值相加放入公式
- MultiplyMods：属性的所有 **有持续时间的** 乘法修改器，计算公式中会把所有修改器的值相加放入公式
- DivideMods：属性的所有 **有持续时间的** 除法修改器，如果该数组为空，则计算公式中，DivideMods为1
- InnerOverrideMods：属性的所有 **有持续时间的** 内部覆盖修改器，如果数组不为空，则数组的第一个修改器的值会替代 **BaseValue** 作为 **BaseValueToUse**，且此时BaseValue不可被修改
- OuterOverrideMods：属性的所有 **有持续时间的** 外部覆盖修改器，如果数组不为空，则数组的第一个修改器的值会直接作为 **CurrentValue**

### 属性CurrentValue的计算公式

$$
CurrentValue = \frac {(BaseValueToUse + PlusMods - MinusMods) * (1 + MultiplyMods)} {DivideMods}
$$

### 属性的范围夹值

*FireflyAttribute.h*
```C++
protected:
	/** 如果为true，如果为true，则属性值至少不能小于 LessBaseValue */
	UPROPERTY(EditDefaultsOnly, Category = "ClampRange")
	bool bAttributeMustNotLessThanSelection = true;

	/** bAttributeMustMoreNotLessThanSelection为true时，属性不会小于该值 */
	UPROPERTY(EditDefaultsOnly, Category = "ClampRange", Meta = (EditCondition = "bAttributeMustNotLessThanSelection"))
	float LessBaseValue = 0.f;

	/** 属性是否需要夹值 */
	UPROPERTY(EditDefaultsOnly, Category = "ClampRange")
	bool bAttributeHasRange = false;

	/** 属性的范围最小值 */
	UPROPERTY(EditDefaultsOnly, Category = "ClampRange", Meta = (EditCondition = "bAttributeHasRange"))
	float RangeMinValue = 0.f;

	/** 属性的范围最大值 */
	UPROPERTY(EditDefaultsOnly, Category = "ClampRange", Meta = (EditCondition = "bAttributeHasRange"))
	float RangeMaxValue = 0.f;

	/** 属性的范围最大值属性类型，基于另一个属性的当前值，当且仅当所属的技能管理器中由该类型的属性时生效 */
	UPROPERTY(EditDefaultsOnly, Category = "ClampRange", Meta = (EditCondition = "bAttributeHasRange"))
	TEnumAsByte<EFireflyAttributeType> RangeMaxValueType = AttributeType_Default;
```

### 属性的修改器

*FireflyAttribute.h*
```C++
USTRUCT()
struct FFireflyAttributeModifier
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	UObject* ModSource = nullptr;

	UPROPERTY()
	float ModValue = 0.f;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY()
	bool bIsActive = true;

	FFireflyAttributeModifier() {}

	FFireflyAttributeModifier(UObject* InSource, float InValue) : ModSource(InSource), ModValue(InValue) {}

	FFireflyAttributeModifier(UObject* InSource, float InValue, int32 InStack) : ModSource(InSource), ModValue(InValue), StackCount(InStack) {}

	FORCEINLINE bool operator==(const FFireflyAttributeModifier& Other) const
	{
		return ModSource == Other.ModSource && ModValue == Other.ModValue;
	}
};
```
## 函数接口

_**注：技能系统管理器组件是属性的友元类**_

### 仅属性内部使用的接口

如下接口仅可以在属性内部的C++类或蓝图类中调用，外部不可调用：

*FireflyAttribute.h*
```C++
protected:

#pragma region Attribute_Basic 属性基础

	/** 初始化属性实例，当属性被构建并添加到技能管理器时触发 */
	UFUNCTION()
	virtual void InitAttributeInstance();

	/** 蓝图端实现的初始化属性实例，当属性被构建并添加到技能管理器时触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Attribute", Meta = (DisplayName = "InitAttributeInstance"))
	void ReceiveInitAttributeInstance();

	/** 获取属性的类型 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE TEnumAsByte<EFireflyAttributeType> GetAttributeType() const;

	/** 获取属性的范围最大值属性类型，基于另一个属性的当前值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE TEnumAsByte<EFireflyAttributeType> GetRangeMaxValueType() const;

	/** 获取属性的当前值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetCurrentValue() const;

	/** 获取属性的基础值或内部覆盖修改器的最新值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetBaseValueToUse() const;

	/** 获取属性所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取属性所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyAbilitySystemComponent* GetOwnerManager() const;

	/** 初始化属性 */
	void InitializeAttributeValue(float InitValue);

#pragma endregion


#pragma region Attribute_Modifier 属性修改器

	/** 更新属性的当前值 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Attribute")
	void UpdateCurrentValue();

	/** 更新属性的当前值 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Attribute")
	void UpdateBaseValue(EFireflyAttributeModOperator ModOperator, float ModValue);

	/** 获取属性的加法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalPlusModifier() const;

	/** 获取属性的减法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalMinusModifier() const;

	/** 获取属性的乘法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalMultiplyModifier() const;

	/** 获取属性的除法修改器的合值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE float GetTotalDivideModifier() const;

	/** 获取属性的内部覆盖修改器的最新值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = "true"))
	FORCEINLINE bool GetNewestOuterOverrideModifier(float& NewestValue) const;

#pragma endregion
```

### 技能系统管理器关于属性的接口

如下接口主要设计属性值的获取、属性实例的构建、属性的初始化三个方面：

*FireflyAbilitySystemComponent.h*
```C++
protected:
	/** 根据属性类型获取属性实例 */
	UFUNCTION()
	UFireflyAttribute* GetAttributeByType(EFireflyAttributeType AttributeType) const;

	/** 根据属性名获得属性实例 */
	UFUNCTION()
	UFireflyAttribute* GetAttributeByName(FName AttributeName) const;

public:
	/** 通过属性标签获取一个属性的当前值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	float GetAttributeValue(EFireflyAttributeType AttributeType) const;

	/** 通过属性标签获取一个属性的基础值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	float GetAttributeBaseValue(EFireflyAttributeType AttributeType) const;

	/** 通过构造器设置构造属性并添加到属性修改器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ConstructAttributeByConstructor(FFireflyAttributeConstructor AttributeConstructor);

	/** 通过属性类型构造属性并添加到属性修改器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ConstructAttributeByClass(TSubclassOf<UFireflyAttribute> AttributeClass);

	/** 通过属性类型构造属性并添加到属性修改器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ConstructAttributeByType(EFireflyAttributeType AttributeType);

	/** 根据类型初始化属性值 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void InitializeAttributeByType(EFireflyAttributeType AttributeType, float NewInitValue);

	/** 根据名称初始化属性值 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void InitializeAttributeByName(FName AttributeName, float NewInitValue);
```

如下接口主要涉及属性修改器的应用和移除：

*FireflyAbilitySystemComponent.h*
```C++
protected:
	/** 修改器被应用前处理的逻辑 */
	UFUNCTION()
	virtual void PreModiferApplied(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);

	/** 修改器被应用后处理的逻辑 */
	UFUNCTION()
	virtual void PostModiferApplied(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);

public:
	/** 应用一个修改器到某个属性的当前值中，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	virtual void ApplyModifierToAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);

	/** 移除某个作用于某个属性的当前值的修改器，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	virtual void RemoveModifierFromAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

	/** 检验某个属性修改器是否可以被应用，该函数仅考虑属性的值被修改器修改后是否仍处于属性的价值范围内，所以要被检验的属性必须是被夹值的 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	virtual bool CanApplyModifierInstant(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, float ModValue) const;

	/** 应用一个修改器永久修改某个属性的基础值，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	virtual void ApplyModifierToAttributeInstant(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

	/** 应用或重新设置一个修改器到某个属性的当前值中必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	virtual void ApplyOrResetModifierToAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);
```

### 技能系统函数库关于属性的接口

如下接口主要是Gameplay开发过程中关于属性的辅助函数：

*FireflyAbilitySystemLibrary.h*
```C++
/** 返回一个属性实例的名称 */
static FString GetAttributeTypeName(EFireflyAttributeType AttributeType);

/** 获取Actor的某个属性的当前值，如果不存在，返回0 */
UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
static float GetAttributeValue(const AActor* Actor, EFireflyAttributeType AttributeType);

/** 获取Actor的某个属性的基础值，如果不存在，返回0 */
UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
static float GetAttributeBaseValue(const AActor* Actor, EFireflyAttributeType AttributeType);
```

### 技能实例内部关于属性的接口

如下接口主要是编写技能逻辑的过程中使用的关于属性的辅助函数，仅能在技能实例内部调用：

*FireflyAbility.h*
```C++
protected:
	/** 获取拥有者的特定属性的值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = true))
	float GetOwnerAttributeValue(EFireflyAttributeType AttributeType) const;

	/** 获取拥有者的特定属性的基础值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute", Meta = (BlueprintProtected = true))
	float GetOwnerAttributeBaseValue(EFireflyAttributeType AttributeType) const;
```

## 代理委托

和属性相关的代理委托只有两个，分别在某个属性的当前值更新和基础值更新时广播：

*FireflyAbilitySystemComponent.h*
```C++
public:
	/** 属性的当前值更新时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Attribute")
	FFireflyAttributeValueChangeDelegate OnAttributeValueChanged;

	/** 属性的基础值更新时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Attribute")
	FFireflyAttributeValueChangeDelegate OnAttributeBaseValueChanged;
```

## 属性范式

技能系统除了属性基类意外，还提供了两种属性类型范式，两种属性类在插件Content文件夹中：
+ FAtt_DerivedAttribute：参考Dota2，战斗单位每增加1点力量值，其最大生命值也额外获得20点，即力量为基础属性，最大生命值为派生属性。在该属性类的蓝图类中，需要设置 **BaseAttributeType(基础属性类型)** 和 **DerivedAttributeCoefficient(派生属性更新参数)**，依旧拿Dota2举例，该属性本身应该是“最大生命值”，其基础属性类型应该是“力量”，派生属性更新参数值应该为“20”
+ FAtt_DynamicRangeMax：如生命值、魔法值这类会经常被消耗，且受动态的范围限制的属性，推荐让这类属性的当前值和最大值分别作为两个属性存在，并让这类属性的当前值继承自该蓝图类

# 效果模块：Effect

## 基础概念

### 效果的持续性

|持续性策略|说明|
|----|----|
|Instant|<ul><li>该类型的效果会在被应用后立刻执行一次，然后结束并销毁自己</li><li>该类型的效果适合执行伤害判断、生命值回复等逻辑</li></ul>|
|Infinite|<ul><li>该类型的效果会在被应用后开始执行，并永久存在，直到被手动移除，才会结束执行并销毁自己</li><li>该类型的效果适合执行场景触发类Buff，比如进入某领域移动速度降低</li></ul>|
|HasDuration|<ul><li>该类型的效果会在被应用后开始执行并计时，当计时器到达设置的持续时间，则结束执行并销毁自己</li><li>该类型的效果适合执行短暂有效的Buff，比如持续3秒的禁止释放技能Buff</li><li>持续时间可以在效果类中设置，或者在应用效果前利用函数设置</li></ul>|

### 效果的周期性跳动执行

+ 周期性跳动执行，即给定一个时间间隔x，每隔x秒，执行一次效果的逻辑。如某个Buff效果，持续5秒，每秒为英雄回复50点生命值
+ 只有持续性策略为 **Infinite** 和 **HasDuration** 的效果可以设置周期性跳动执行
+ 当且仅当 **bIsEffectExecutionPeriodic** 为true时，效果才会在被应用后开启周期性跳动执行，但如果 **PeriodicInterval** 小于等于0，则周期性跳动执行依旧无效

### 效果的堆叠管理

+ StackingPolicy：效果要选择的堆叠管理策略

|堆叠管理策略|说明|
|----|----|
|None|该效果不会堆叠|
|StackNoLimit|该效果会堆叠，并且堆叠无上限|
|StackHasLimit|该效果会堆叠，但当该效果堆叠数达到执行上限，会执行其他逻辑|

+ StackingLimitation：效果的堆叠数上限，仅在堆叠策略为 **StackHasLimit** 时起作用

效果模块需要增加代码，可以动态设置效果的堆叠管理，以及检验StackingLimitation是否为0
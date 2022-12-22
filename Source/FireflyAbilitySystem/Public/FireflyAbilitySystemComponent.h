// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FireflyAbilitySystemTypes.h"
#include "FireflyAbility.h"
#include "FireflyEffect.h"
#include "FireflyAttribute.h"
#include "FireflyAbilitySystemComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;

/** 输入和技能绑定的数据 */
USTRUCT()
struct FFireflyAbilitiesBoundToInput
{
	GENERATED_USTRUCT_BODY()

public:
	/** 输入绑定的所有技能 */
	UPROPERTY()
	TArray<TSubclassOf<UFireflyAbility>> Abilities = TArray<TSubclassOf<UFireflyAbility>>{};

	/** 输入事件句柄：开始 */
	UPROPERTY()
	uint32 HandleStarted = -1;

	/** 输入事件句柄：执行中 */
	UPROPERTY()
	uint32 HandleOngoing = -1;

	/** 输入事件句柄：取消 */
	UPROPERTY()
	uint32 HandleCanceled = -1;

	/** 输入事件句柄：触发 */
	UPROPERTY()
	uint32 HandleTriggered = -1;

	/** 输入事件句柄：完成 */
	UPROPERTY()
	uint32 HandleCompleted = -1;

	FFireflyAbilitiesBoundToInput()	{}

	FORCEINLINE bool operator==(const FFireflyAbilitiesBoundToInput& Other)
	{
		return Abilities == Other.Abilities
			&& HandleStarted == Other.HandleStarted
			&& HandleOngoing == Other.HandleOngoing
			&& HandleCanceled == Other.HandleCanceled
			&& HandleTriggered == Other.HandleTriggered
			&& HandleCompleted == Other.HandleCompleted;
	}
};

/** 技能执行周期的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFireflyAbilityExecutionDelegate, FName, AbilityID, TSubclassOf<UFireflyAbility>, AbilityType);
/** 技能执行冷却的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFireflyAbilityCooldownExecutionDelegate, FName, AbilityID, TSubclassOf<UFireflyAbility>, AbilityType, float, TotoalDuration);
/** 技能冷却的剩余时间被更新的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFireflyAbilityCooldownRemainingChangedDelegate, FName, AbilityID, TSubclassOf<UFireflyAbility>, AbilityType, float, NewTimeRemaining, float, TotalDuration);

/** 属性数值变更的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFireflyAttributeValueChangeDelegate, TEnumAsByte<EFireflyAttributeType>, AttributeType, float, NewValue, float, OldValue);

/** 效果执行开始的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFireflyEffectStartExecutingDelegate, FName, EffectID, TSubclassOf<UFireflyEffect>, EffectType, float, TotoalDuration);
/** 效果执行结束的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFireflyEffectEndExecutingDelegate, FName, EffectID, TSubclassOf<UFireflyEffect>, EffectType);
/** 效果的剩余持续时间被更新的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFireflyEffectTimeRemainingChangedDelegate, FName, EffectID, TSubclassOf<UFireflyEffect>, EffectType, float, NewTimeRemaining, float, TotalDuration);
/** 效果的堆叠数被更新的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFireflyEffectStackingChangedDelegate, FName, EffectID, TSubclassOf<UFireflyEffect>, EffectType, int32, NewStackCount, int32, OldStackCount);

/** Tag存在周期的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFireflyGameplayTagExecutionDelegate, FGameplayTagContainer, TagsUpdated);

/** 处理消息事件的代理声明 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFireflyMessageEventDelegate, FGameplayTag, EventTag, FFireflyMessageEventData, EventData);

/** 技能系统管理器的组件 */
UCLASS( ClassGroup = (FireflyAbilitySystem), HideCategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), Meta = (BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyAbilitySystemComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

#pragma region Override // 基类重载

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion


#pragma region Basic // 基础

protected:
	/** 管理器的拥有者是否拥有权威权限 */
	virtual bool HasAuthority() const;

	/** 组件拥有者是否拥有本地控制权限 */
	virtual bool IsLocallyControlled() const;

#pragma endregion


#pragma region Ability_Granting // 技能赋予

protected:
	/** 根据ID获取一个该管理器中的相关技能实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability")
	FORCEINLINE UFireflyAbility* GetGrantedAbilityByID(FName AbilityID) const;

	/** 根据Tag获取所有该管理器中的相关技能实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability")
	FORCEINLINE TArray<UFireflyAbility*> GetGrantedAbilityByTag(FGameplayTag AbilityTag) const;

	/** 根据类型获取一个该管理器中的相关技能实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability")
	FORCEINLINE UFireflyAbility* GetGrantedAbilityByClass(TSubclassOf<UFireflyAbility> AbilityType) const;

public:
	/** 根据ID为技能管理器赋予一个技能，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	virtual bool GrantAbilityByID(FName AbilityID);

	/** 根据类型为技能管理器赋予一个技能，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	virtual bool GrantAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToGrant, FName AbilityID = NAME_None);

	/** 根据ID从技能管理器移除一个技能，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	virtual void RemoveAbilityByID(FName AbilityID, bool bRemoveOnEnded);

	/** 根据类型从技能管理器移除一个技能，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	virtual void RemoveAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToRemove, bool bRemoveOnEnded);

protected:
	/** 技能管理器被赋予的技能 */
	UPROPERTY(Replicated)
	TArray<UFireflyAbility*> GrantedAbilities;

#pragma endregion


#pragma region Ability_Execution // 技能执行

protected:
	/** 尝试激活技能，内部执行 */
	UFUNCTION()
	void TryActivateAbilityInternal(UFireflyAbility* Ability);

	/** 激活技能，内部执行 */
	UFUNCTION()
	void ActivateAbilityInternal(UFireflyAbility* Ability);

	/** 服务端尝试激活技能 */
	UFUNCTION(Server, Reliable)
	void Server_TryActivateAbility(UFireflyAbility* Ability);

	/** 服务端通知本地客户端激活技能  */
	UFUNCTION(Client, Reliable)
	void Client_ActivateAbility(UFireflyAbility* Ability);

public:
	/** 尝试通过ID激活并执行技能逻辑，该函数会尝试在本地客户端激活技能，在服务端进行二次验证 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	virtual bool TryActivateAbilityByID(FName AbilityID);

	/** 尝试通过类型激活并执行技能逻辑，该函数会尝试在本地客户端激活技能，在服务端进行二次验证 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	virtual bool TryActivateAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToActivate);

	/** 获取所有正在激活的技能 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability")
	FORCEINLINE TArray<UFireflyAbility*> GetActivatingAbilities() const { return ActivatingAbilities; }

	/** 取消所有带有特定资产Tag的技能的激活状态，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void CancelAbilitiesWithTags(FGameplayTagContainer CancelTags);

	/** 某个技能结束执行时执行的函数 */
	UFUNCTION()
	virtual void OnAbilityEndActivation(UFireflyAbility* AbilityJustEnded);

	/** 更改某个技能的当前冷却剩余时间，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void SetAbilityCooldownRemaining(TSubclassOf<UFireflyAbility> AbilityType, float NewTimeRemaining);

protected:
	/** 所有激活中的运行中的技能 */
	UPROPERTY()
	TArray<UFireflyAbility*> ActivatingAbilities;

public:
	/** 技能激活时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FFireflyAbilityExecutionDelegate OnAbilityActivated;

	/** 技能结束时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FFireflyAbilityExecutionDelegate OnAbilityEnded;

	/** 技能取消时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FFireflyAbilityExecutionDelegate OnAbilityCanceled;

	/** 当技能的消耗执行成功时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FFireflyAbilityExecutionDelegate OnAbilityCostCommitted;

	/** 当技能的冷却执行成功时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FFireflyAbilityCooldownExecutionDelegate OnAbilityCooldownCommitted;

	/** 当技能的冷却剩余时间改变时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FFireflyAbilityCooldownRemainingChangedDelegate OnAbilityCooldownRemainingChanged;

#pragma endregion


#pragma region Ability_Requirement // 技能释放条件

protected:
	/** 获取管理器当前会阻挡激活的技能资产Tags */
	UFUNCTION()
	FGameplayTagContainer GetBlockAbilityTags() const;

public:
	/** 更新管理器的阻断技能Tags，或当CancelTags生效时取消某些技能，仅当某个技能激活时才会触发 */
	UFUNCTION()
	void UpdateBlockAndCancelAbilityTags(FGameplayTagContainer BlockTags, FGameplayTagContainer CancelTags, bool bIsActivated);

protected:
	/** 携带这些资产Tag的技能会被阻拦激活 */
	UPROPERTY()
	TMap<FGameplayTag, int32> BlockAbilityTags;

#pragma endregion


#pragma region Ability_InputBinding // 技能输入绑定

protected:
	/** 从组件拥有者身上获取增强输入组件 */
	UFUNCTION()
	FORCEINLINE UEnhancedInputComponent* GetEnhancedInputComponentFromOwner() const;

public:
	/** 将技能与输入绑定，绑定操作应该在本地客户端执行，输入必须有效，如技能未在管理器中，则先给输入创建绑定 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void BindAbilityToInput(TSubclassOf<UFireflyAbility> AbilityToBind, UInputAction* InputToBind);

	/** 将技能与输入解绑，解绑操作应该在本地客户端执行，技能需要存在于技能管理器中，输入也应当有效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	void UnbindAbilityWithInput(TSubclassOf<UFireflyAbility> AbilityToUnbind, UInputAction* InputToUnbind);

protected:
	/** 所有和输入绑定了的技能 */
	UPROPERTY()
	TMap<UInputAction*, FFireflyAbilitiesBoundToInput> AbilitiesInputBound;

#pragma endregion


#pragma region Ability_InputEvent // 技能输入事件

protected:
	/** 组件管理的输入事件触发：开始 */
	UFUNCTION()
	virtual void OnAbilityInputActionStarted(UInputAction* Input);

	/** 组件管理的输入事件触发：等待触发 */
	UFUNCTION()
	virtual void OnAbilityInputActionOngoing(UInputAction* Input);

	/** 组件管理的输入事件触发：取消 */
	UFUNCTION()
	virtual void OnAbilityInputActionCanceled(UInputAction* Input);

	/** 组件管理的输入事件触发：触发 */
	UFUNCTION()
	virtual void OnAbilityInputActionTriggered(UInputAction* Input);

	/** 组件管理的输入事件触发：完成 */
	UFUNCTION()
	virtual void OnAbilityInputActionCompleted(UInputAction* Input);

#pragma endregion


#pragma region Attribute_Basic // 属性基础

protected:
	/** 根据属性类型获取属性实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	UFireflyAttribute* GetAttributeByType(EFireflyAttributeType AttributeType) const;

	/** 根据属性名获得属性实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
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

protected:
	/** 属性容器 */
	UPROPERTY(Replicated)
	TArray<UFireflyAttribute*> AttributeContainer;

public:
	/** 属性的当前值更新时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Attribute")
	FFireflyAttributeValueChangeDelegate OnAttributeValueChanged;

	/** 属性的基础值更新时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Attribute")
	FFireflyAttributeValueChangeDelegate OnAttributeBaseValueChanged;

#pragma endregion


#pragma region Attribute_Modifier // 属性修改器

protected:
	UFUNCTION()
	virtual void PostModiferApplied(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);

public:
	/** 应用一个修改器到某个属性的当前值中，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyModifierToAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);

	/** 移除某个作用于某个属性的当前值的修改器，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveModifierFromAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

	/** 检验某个属性修改器是否可以被应用，该函数仅考虑属性的值被修改器修改后是否仍处于属性的价值范围内，所以要被检验的属性必须是被夹值的 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	bool CanApplyModifierInstant(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, float ModValue) const;

	/** 应用一个修改器永久修改某个属性的基础值，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyModifierToAttributeInstant(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

	/** 应用一个修改器到某个属性的当前值中，该修改器必须来自于属性本身，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyModifierToAttributeSelf(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);

#pragma endregion


#pragma region Effect_Application // 效果应用

public:
	/** 在该管理器中获取特定ID的被应用的激活中的所有效果 */
	UFUNCTION()
	TArray<UFireflyEffect*> GetActiveEffectsByID(FName EffectID) const;

	/** 在该管理器中获取特定类型的被应用的激活中的所有效果 */
	UFUNCTION()
	TArray<UFireflyEffect*> GetActiveEffectsByClass(TSubclassOf<UFireflyEffect> EffectType) const;

	/** 在该管理器中获取带有特定资产Tags的被应用的激活中的所有效果 */
	UFUNCTION()
	TArray<UFireflyEffect*> GetActiveEffectsByTag(FGameplayTagContainer EffectAssetTags) const;

	/** 获取管理器当前会阻挡激活的技能资产Tags */
	UFUNCTION()
	FGameplayTagContainer GetBlockEffectTags() const;

	/** 为自身应用一个效果实例或应用效果的固定堆叠数，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToOwner(AActor* Instigator, UFireflyEffect* EffectInstance, int32 StackToApply = 1);

	/** 为目标应用一个效果实例或应用效果的固定堆叠数，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToTarget(AActor* Target, UFireflyEffect* EffectInstance, int32 StackToApply = 1);

	/** 为自身应用效果或应用效果的固定堆叠数，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToOwnerByID(AActor* Instigator, FName EffectID, int32 StackToApply = 1);

	/** 为目标应用效果或应用效果的固定堆叠数，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToTargetByID(AActor* Target, FName EffectID, int32 StackToApply = 1);

	/** 为自身应用效果或应用效果的固定堆叠数，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToOwnerByClass(AActor* Instigator, TSubclassOf<UFireflyEffect> EffectType, FName EffectID = NAME_None, int32 StackToApply = 1);

	/** 为目标应用效果或应用效果的固定堆叠数，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToTargetByClass(AActor* Target, TSubclassOf<UFireflyEffect> EffectType, FName EffectID = NAME_None, int32 StackToApply = 1);

	/** 为目标应用一个根据动态构造器实现的效果，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectDynamicConstructorToOwner(AActor* Instigator, FFireflyEffectDynamicConstructor EffectSetup, int32 StackToApply = 1);

	/** 为目标应用一个根据动态构造器实现的效果，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectDynamicConstructorToTarget(AActor* Target, FFireflyEffectDynamicConstructor EffectSetup, int32 StackToApply = 1);

	/** 一处自身特定的所有效果的固定堆叠数, StackToRemove = -1时，移除所有效果的所有堆叠，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void RemoveActiveEffectByID(FName EffectID, int32 StackToRemove = -1);

	/** 移除自身特定的所有效果的固定堆叠数, StackToRemove = -1时，移除所有效果的所有堆叠，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void RemoveActiveEffectByClass(TSubclassOf<UFireflyEffect> EffectType, int32 StackToRemove = -1);

	/** 移除所有带有特定资产Tag的效果的应用状态，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void RemoveActiveEffectsWithTags(FGameplayTagContainer RemoveTags);	

	/** 将某个效果从ActiveEffects中添加或删除，必须在拥有权限端执行，否则无效 */
	UFUNCTION()
	void AddOrRemoveActiveEffect(UFireflyEffect* InEffect, bool bIsAdd);

	/** 更新管理器的阻断技能Tags，或当CancelTags生效时取消某些效果，仅当某个不为Instant的效果被应用时才会触发，必须在拥有权限端执行，否则无效 */
	UFUNCTION()
	void UpdateBlockAndRemoveEffectTags(FGameplayTagContainer BlockTags, FGameplayTagContainer RemoveTags, bool bIsApplied);

protected:
	/** 所有激活中的执行策略不是Instant的效果 */
	UPROPERTY(Replicated)
	TArray<UFireflyEffect*> ActiveEffects;

	/** 携带这些资产Tag的技能会被阻拦激活 */
	UPROPERTY()
	TMap<FGameplayTag, int32> BlockEffectTags;

public:
	/** 当不为Instant的效果被应用时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Effect")
	FFireflyEffectStartExecutingDelegate OnActiveEffectApplied;

	/** 当不为Instant的效果被移除时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Effect")
	FFireflyEffectEndExecutingDelegate OnActiveEffectRemoved;

#pragma endregion


#pragma region Effect_Duration // 效果持续时间

public:
	/** 获取某种效果的剩余作用时间和总持续时间，若该种效果在管理器中同时存在多个，默认取第一个 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	bool GetSingleActiveEffectTimeDurationByID(FName EffectID, float& TimeRemaining, float& TotalDuration) const;

	/** 获取某种效果的剩余作用时间和总持续时间，若该种效果在管理器中同时存在多个，默认取第一个 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	bool GetSingleActiveEffectTimeDurationByClass(TSubclassOf<UFireflyEffect> EffectType, float& TimeRemaining, float& TotalDuration) const;

	/** 设置某种效果的剩余作用时间，若该种效果在管理器中同时存在多个，默认取第一个，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void SetSingleActiveEffectTimeRemaining(TSubclassOf<UFireflyEffect> EffectType, float NewTimeRemaining);

public:
	/** 某个效果的剩余持续时间更新时触发的代理 */
	UPROPERTY(BlueprintAssignable)
	FFireflyEffectTimeRemainingChangedDelegate OnEffectTimeRemainingChanged;

#pragma endregion


#pragma region Effect_Stacking // 效果堆叠

public:
	/** 获取某种效果的当前堆叠数，若该种效果在管理器中同时存在多个，默认取第一个 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	bool GetSingleActiveEffectStackingCountByID(FName EffectID, int32& StackingCount) const;

	/** 获取某种效果的当前堆叠数，若该种效果在管理器中同时存在多个，默认取第一个 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	bool GetSingleActiveEffectStackingCountByClass(TSubclassOf<UFireflyEffect> EffectType, int32& StackingCount) const;

public:
	/** 当不为Instant的效果的堆叠数发生变化时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Effect")
	FFireflyEffectStackingChangedDelegate OnEffectStackingChanged;

#pragma endregion


#pragma region Effect_Dynamic // 动态效果

public:
	/** 根据ID动态构建一个效果实例 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* MakeDynamicEffectByID(FName EffectID);

	/** 根据类型动态构建一个效果实例 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* MakeDynamicEffectByClass(TSubclassOf<UFireflyEffect> EffectType, FName EffectID = NAME_None);

	/** 为动态创建的效果实例添加资产Tags */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* AssignDynamicEffectAssetTags(UFireflyEffect* EffectInstance, FGameplayTagContainer NewEffectAssetTags);

	/** 为动态创建的效果实例添加赋予给Owner的Tags */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* AssignDynamicEffectGrantTags(UFireflyEffect* EffectInstance, FGameplayTagContainer NewEffectGrantTags);

	/** 为动态创建的效果实例设置持续时间 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* SetDynamicEffectDuration(UFireflyEffect* EffectInstance, float Duration);

	/** 为动态创建的效果实例设置周期性执行的间隔 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* SetDynamicEffectPeriodicInterval(UFireflyEffect* EffectInstance, float PeriodicInterval);

	/** 为动态创建的效果实例设置属性修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* AssignDynamicEffectModifiers(UFireflyEffect* EffectInstance, FFireflyEffectModifierData NewModifier);

	/** 为动态创建的效果实例设置属性修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	UFireflyEffect* AssignDynamicEffectSpecificProperties(UFireflyEffect* EffectInstance, FFireflySpecificProperty NewSpecificProperty);

#pragma endregion


#pragma region Effect_Modifier // 效果修改器

public:
	/** 为管理器添加一些特殊属性 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void AppendEffectSpecificProperties(TArray<FFireflySpecificProperty> InSpecificProperties);

	/** 从管理器中移除一些特殊属性 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void RemoveEffectSpecificProperties(TArray<FFireflySpecificProperty> InSpecificProperties);

	/** 获取管理器当前携带的特殊属性集合 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	FORCEINLINE TArray<FFireflySpecificProperty> GetSpecificProperties() const { return SpecificProperties; }

protected:
	/** 被效果赋予的特殊属性集合 */
	UPROPERTY()
	TArray<FFireflySpecificProperty> SpecificProperties;

#pragma endregion


#pragma region TagManagement // 标签管理

public:
	/** 获取拥有的所有Tag */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Tag")
	FGameplayTagContainer GetContainedTags() const;

	/** 添加Tag到管理器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Tag")
	void AddTagToManager(FGameplayTag TagToAdd, int32 CountToAdd = 1);

	/** 从管理器中移除Tag */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Tag")
	void RemoveTagFromManager(FGameplayTag TagToRemove, int32 CountToRemove = 1);

	/** 将一些Tag添加到管理器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Tag")
	void AddTagsToManager(FGameplayTagContainer TagsToAdd, int32 CountToAdd = 1);

	/** 将一些Tag从管理器中移除 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Tag")
	void RemoveTagsFromManager(FGameplayTagContainer TagsToRemove, int32 CountToRemove = 1);

protected:
	/** 所有拥有的Tag及其对应的堆叠数 */
	UPROPERTY()
	TMap<FGameplayTag, int32> TagCountContainer;

public:
	/** 管理器的TagCountContainer更新时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Tag")
	FFireflyGameplayTagExecutionDelegate OnTagContainerUpdated;

#pragma endregion


#pragma region MessageEvent // 消息事件

protected:
	/** 尝试通过消息事件触发技能激活 */
	UFUNCTION()
	virtual void TryTriggerAbilityByMessage(UFireflyAbility* Ability, const FFireflyMessageEventData EventData);

	/** 通过消息事件触发技能激活 */
	UFUNCTION()
	virtual void TriggerAbilityByMessage(UFireflyAbility* Ability, const FFireflyMessageEventData EventData);

	/** 服务端尝试通过消息事件触发技能激活 */
	UFUNCTION(Server, Reliable)
	virtual void Server_TryTriggerAbilityByMessage(UFireflyAbility* Ability, const FFireflyMessageEventData EventData);

	/** 服务端通知本地客户端通过消息事件触发技能激活 */
	UFUNCTION(Client, Reliable)
	virtual void Client_TriggerAbilityByMessage(UFireflyAbility* Ability, const FFireflyMessageEventData EventData);

public:
	/** 处理通知事件 */
	UFUNCTION()
	virtual void HandleMessageEvent(FGameplayTag EventTag, FFireflyMessageEventData EventData);

public:
	UPROPERTY(BlueprintAssignable)
	FFireflyMessageEventDelegate OnReceiveMessageEvent;

#pragma endregion
};

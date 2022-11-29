﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FireflyAbilitySystemTypes.h"
#include "FireflyAbilitySystemComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class UFireflyAbility;
class UFireflyAttribute;

/** 输入和技能绑定的数据 */
USTRUCT()
struct FFireflyAbilitiesBoundToInput
{
	GENERATED_USTRUCT_BODY()

public:
	/** 输入绑定的所有技能 */
	UPROPERTY()
	TArray<TSubclassOf<UFireflyAbility>> Abilities;

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

	FFireflyAbilitiesBoundToInput() : Abilities(TArray<TSubclassOf<UFireflyAbility>>{})
	{}

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

/** 技能系统管理器的组件 */
UCLASS(ClassGroup = (FireflyAbilitySystem), meta = (BlueprintSpawnableComponent))
class FIREFLYABILITYSYSTEM_API UFireflyAbilitySystemComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

#pragma region Override

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion


#pragma region Ability_Basic

protected:
	/** 组件拥有者是否拥有本地控制权限 */
	UFUNCTION()
	FORCEINLINE bool IsOwnerLocallyControlled() const;

#pragma endregion


#pragma region Ability_Granting

protected:
	/** 根据类型获取一个该管理器中的技能实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability")
	FORCEINLINE UFireflyAbility* GetGrantedAbilityByClass(TSubclassOf<UFireflyAbility> AbilityType) const;

public:
	/** 为技能管理器赋予一个技能，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FireflyAbilitySystem|Ability")
	virtual void GrantAbility(TSubclassOf<UFireflyAbility> AbilityToGrant);

	/** 从技能管理器中移除一个技能，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FireflyAbilitySystem|Ability")
	virtual void RemoveAbility(TSubclassOf<UFireflyAbility> AbilityToRemove);

	/** 从技能管理器中移除一个技能，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FireflyAbilitySystem|Ability")
	virtual void RemoveAbilityOnEnded(TSubclassOf<UFireflyAbility> AbilityToRemove);

protected:
	/** 技能管理器被赋予的技能 */
	UPROPERTY(Replicated)
	TArray<UFireflyAbility*> GrantedAbilities;

#pragma endregion


#pragma region Ability_Execution

protected:
	/** 服务器激活技能，可能会验证技能是否真的可以执行 */
	UFUNCTION(Server, Reliable)
	void Server_TryActivateAbility(UFireflyAbility* AbilityToActivate, bool bNeedValidation);

public:
	/** 尝试通过类型激活并执行技能逻辑，该函数若在本地先行检测技能是否可激活，则服务端可能会进行二次验证 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability")
	virtual UFireflyAbility* TryActivateAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToActivate);

	/** 获取所有正在激活的技能 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability")
	FORCEINLINE TArray<UFireflyAbility*> GetActivatingAbilities() const { return ActivatingAbilities; }

	/** 取消所有带有特定资产Tag的技能的激活状态，必须在拥有权限端执行，否则无效 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FireflyAbilitySystem|Ability")
	void CancelAbilitiesWithTags(FGameplayTagContainer CancelTags);

	/** 某个技能结束执行时执行的函数 */
	UFUNCTION()
	virtual void OnAbilityEndActivation(UFireflyAbility* AbilityJustEnded);

protected:
	/** 所有激活中的运行中的技能 */
	UPROPERTY()
	TArray<UFireflyAbility*> ActivatingAbilities;

#pragma endregion


#pragma region Ability_Requirement

public:
	/** 更新管理器的阻断技能Tags，或当CancelTags生效时取消某些技能，仅当某个技能激活时才会触发 */
	UFUNCTION()
	void UpdateBlockAndCancelAbilityTags(FGameplayTagContainer BlockTags, FGameplayTagContainer CancelTags, bool bIsActivated);

protected:
	/** 携带这些资产Tag的技能会被阻拦激活 */
	UPROPERTY()
	TMap<FGameplayTag, int32> BlockAbilityTags;

#pragma endregion


#pragma region Ability_InputBinding

protected:
	/** 从组件拥有者身上获取增强输入组件 */
	UFUNCTION()
	FORCEINLINE UEnhancedInputComponent* GetEnhancedInputComponentFromOwner() const;

public:
	/** 将技能与输入绑定，绑定操作应该在本地客户端执行，技能需要存在于技能管理器中，输入也应当有效 */
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


#pragma region Ability_InputEvent

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


#pragma region Attribute_Basic

protected:
	/** 构造并返回一个属性实例的名称 */
	FString GetAttributeTypeName(EFireflyAttributeType AttributeType) const;

public:
	/** 根据属性类型获取属性实例 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	UFireflyAttribute* GetAttributeByType(EFireflyAttributeType AttributeType) const;	

public:
	/** 通过属性标签获取一个属性的当前值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	float GetAttributeValue(EFireflyAttributeType AttributeType) const;

	/** 通过属性标签获取一个属性的基础值 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Attribute")
	float GetAttributeBaseValue(EFireflyAttributeType AttributeType) const;

	/** 通过类构造属性并添加到属性修改器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ConstructAttributeByClass(TSubclassOf<UFireflyAttribute> AttributeToConstruct, EFireflyAttributeType AttributeType);

	/** 通过属性类型构造属性并添加到属性修改器中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ConstructAttributeByType(EFireflyAttributeType AttributeType);

	/** 初始化属性值 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void InitializeAttribute(EFireflyAttributeType AttributeType, float NewInitValue);

protected:
	/** 属性容器 */
	UPROPERTY(Replicated)
	TArray<UFireflyAttribute*> AttributeContainer;

#pragma endregion


#pragma region Attribute_Modifier

public:
	/** 应用一个修改器到某个属性的当前值中 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyModifierToAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply);

	/** 移除某个作用于某个属性的当前值的修改器 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void RemoveModifierFromAttribute(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

	/** 应用一个修改器永久修改某个属性的基础值 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Attribute")
	void ApplyModifierToAttributeInstant(EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue);

#pragma endregion


#pragma region Effect_Application

protected:
	/** 在该管理器中获取特定类型的被应用的激活中的所有效果 */
	UFUNCTION()
	TArray<UFireflyEffect*> GetActiveEffectsByClass(TSubclassOf<UFireflyEffect> EffectType) const;

public:
	/** 为自身应用效果或应用效果的固定堆叠数 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToSelf(AActor* Instigator, TSubclassOf<UFireflyEffect> EffectType, int32 StackToApply = 1);

	/** 为目标应用效果或应用效果的固定堆叠数 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToTarget(AActor* Target, TSubclassOf<UFireflyEffect> EffectType, int32 StackToApply = 1);

	/** 移除自身特定的所有效果的固定堆叠数, StackToRemove = -1时，移除所有效果的所有堆叠 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void RemoveActiveEffectFromSelf(TSubclassOf<UFireflyEffect> EffectType, int32 StackToRemove = -1);

	/** 将某个效果从ActiveEffects中添加或删除 */
	UFUNCTION()
	void AddOrRemoveActiveEffect(UFireflyEffect* InEffect, bool bIsAdd);

protected:
	/** 所有激活中的执行策略不是Instant的效果 */
	UPROPERTY(Replicated)
	TArray<UFireflyEffect*> ActiveEffects;

#pragma endregion


#pragma region Effect_Duration

public:
	/** 获取某种效果的剩余作用时间和总持续时间，若该种效果在管理器中同时存在多个，默认取第一个 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	bool GetSingleActiveEffectTimeDuration(TSubclassOf<UFireflyEffect> EffectType, float& TimeRemaining, float& TotalDuration) const;

#pragma endregion


#pragma region Effect_Stacking

public:
	/** 获取某种效果的当前堆叠数，若该种效果在管理器中同时存在多个，默认取第一个 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	bool GetSingleActiveEffectStackingCount(TSubclassOf<UFireflyEffect> EffectType, int32& StackingCount) const;

#pragma endregion


#pragma region TagManagement

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

protected:
	/** 所有拥有的Tag及其对应的堆叠数 */
	UPROPERTY()
	TMap<FGameplayTag, int32> TagCountContainer;

#pragma endregion
};
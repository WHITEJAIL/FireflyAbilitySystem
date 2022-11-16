// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "FireflyAbility.generated.h"

class UFireflyAbilityManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAbilityExecutionDelegate);

/** 技能 */
UCLASS(Blueprintable, BlueprintType)
class FIREFLYABILITYSYSTEM_API UFireflyAbility : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Basic

public:
	virtual UWorld* GetWorld() const override;

protected:
	/** 获取技能所属的管理器的拥有者 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	FORCEINLINE AActor* GetOwnerActor() const;

	/** 获取技能所属的管理器组件 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	FORCEINLINE UFireflyAbilityManagerComponent* GetOwnerManager() const;

	friend class UFireflyAbilityManagerComponent;

#pragma endregion


#pragma region Granting

protected:
	/**当技能被赋予时执行的函数，可以理解为技能的运行时构造函数*/
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	void OnAbilityGranted();

protected:
	/** 标识该技能是否应该在某次执行结束后从技能管理器上移除 */
	UPROPERTY()
	bool bRemoveOnEndedExecution = false;

#pragma endregion


#pragma region Execution

protected:
	/** 激活技能 */
	UFUNCTION()
	virtual void ActivateAbility();

	/** 蓝图端的技能激活时的逻辑 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "Actiavate Ability"))
	void ReceiveActivateAbility();

	/** 结束技能 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	virtual void EndAbility();

	/** 取消技能 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	virtual void CancelAbility();

	/** 蓝图端的技能结束时执行的逻辑，分为自动结束和取消结束两种状态 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "End Ability"))
	void ReceiveEndAbility(bool bWasCanceled);	

protected:
	/** 技能激活时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FAbilityExecutionDelegate OnAbilityActivated;

	/** 技能结束时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FAbilityExecutionDelegate OnAbilityEnded;

	/** 技能取消时触发的代理 */
	UPROPERTY(BlueprintAssignable, Category = "FireflyAbilitySystem|Ability")
	FAbilityExecutionDelegate OnAbilityCanceled;

	/** 该技能是否处于激活状态 */
	UPROPERTY()
	bool bIsActivating = false;

#pragma endregion


#pragma region CostAndCooldown

protected:
	/** 检测技能的消耗是否可执行 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	bool CheckAbilityCost() const;

	/** 申请执行技能的消耗 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	void ApplyAbilityCost() const;

	/** 检测技能的冷却是否可执行 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	bool CheckAbilityCooldown() const;

	/** 申请执行技能的冷却 */
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	void ApplyAbilityCooldown() const;

	/** 单独执行技能的消耗 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool CommitAbilityCost();

	/** 单独执行技能的冷却 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool CommitAbilityCooldown();

	/** 执行技能的消耗和冷却 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool CommitAbility();

protected:
	/** 当技能的消耗执行成功时触发的代理 */
	UPROPERTY(BlueprintAssignable)
	FAbilityExecutionDelegate OnAbilityCostCommitted;

	/** 当技能的冷却执行成功时触发的代理 */
	UPROPERTY(BlueprintAssignable)
	FAbilityExecutionDelegate OnAbilityCooldownCommitted;

#pragma endregion


#pragma region Requirement

protected:
	/** 是否可激活技能 */
	UFUNCTION()
	virtual bool CanActivateAbility() const;

	/** 蓝图端的是否可激活技能 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true", DisplayName = "Can Activate Ability"))
	bool ReceiveCanActivateAbility() const;

	bool bHasBlueprintCanActivate;

	/** 当技能激活时，执行该技能的Tag需求关系管理 */
	UFUNCTION()
	void ExecuteTagRequirementOnActivated();

	/** 当技能激活时，执行该技能的Tag需求关系管理 */
	UFUNCTION()
	void ExecuteTagRequirementOnEnded();

protected:
	/** 该技能的激活需要的正在执行的技能，数组中有一个技能正在激活，都可以让该技能激活 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|AbilityRequired")
	TArray<TSubclassOf<UFireflyAbility>> RequiredActivatingAbilities;

	/** 该技能的激活是否会取消其前置需求技能的执行 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|AbilityRequired")
	bool bCancelRequiredActivatingAbilitiesOnActivated;

	/** 为该做特殊技能资产标记的Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsForAbilityAsset;

	/** 该技能的激活执行会取消带有这些资产标记Tags的技能的执行 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsOfAbilitiesWillBeCanceledOnActivated;

	/** 该技能的激活执行会阻断带有这些资产标记Tags的技能的执行 */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsOfAbilitiesWillNotBeActivatedOnActivated;

	/** 该技能激活执行会为Owner添加如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsApplyToOwnerOnActivated;

	/** 该技能激活执行需要技能管理器含有如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsRequireOwnerHasForActivation;

	/** 该技能激活执行期望技能管理器不含如下Tags */
	UPROPERTY(EditDefaultsOnly, Category = "ActivationRequirement|TagRequired")
	FGameplayTagContainer TagsBlockActivationOnOwnerHas;

#pragma endregion
	
};

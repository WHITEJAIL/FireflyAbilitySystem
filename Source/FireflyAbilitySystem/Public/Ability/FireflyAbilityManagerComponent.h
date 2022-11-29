// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FireflyAbilityManagerComponent.generated.h"

class UFireflyAbility;

/** 技能管理器组件 */
UCLASS( ClassGroup = (FireflyAbilitySystem), meta = (BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyAbilityManagerComponent : public UActorComponent
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


#pragma region Granting

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


#pragma region Execution

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


#pragma region Requirement

public:
	/** 更新管理器的阻断技能Tags，或当CancelTags生效时取消某些技能，仅当某个技能激活时才会触发 */
	UFUNCTION()
	void UpdateBlockAndCancelAbilityTags(FGameplayTagContainer BlockTags, FGameplayTagContainer CancelTags, bool bIsActivated);

protected:
	/** 携带这些资产Tag的技能会被阻拦激活 */
	UPROPERTY()
	TMap<FGameplayTag, int32> BlockAbilityTags;

#pragma endregion

};

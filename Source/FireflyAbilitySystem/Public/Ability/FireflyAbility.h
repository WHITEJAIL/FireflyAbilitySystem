// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FireflyAbility.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAbilityExecutionDelegate);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class FIREFLYABILITYSYSTEM_API UFireflyAbility : public UObject
{
	GENERATED_UCLASS_BODY()

#pragma region Execution

protected:
	UFUNCTION()
	virtual void StartAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "Start Ability"))
	void ReceiveStartAbility();

	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	virtual void EndAbility();

	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	virtual void CancelAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "FireflyAbilitySystem|Ability", Meta = (DisplayName = "End Ability"))
	void ReceiveEndAbility(bool bWasCanceled);	

protected:
	UPROPERTY(BlueprintAssignable)
	FAbilityExecutionDelegate OnAbilityStarted;

	UPROPERTY(BlueprintAssignable)
	FAbilityExecutionDelegate OnAbilityEnded;

	UPROPERTY(BlueprintAssignable)
	FAbilityExecutionDelegate OnAbilityCanceled;

#pragma endregion


#pragma region CostAndCooldown

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	bool CheckAbilityCost() const;

	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	void ApplyAbilityCost() const;

	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	bool CheckAbilityCooldown() const;

	UFUNCTION(BlueprintNativeEvent, Category = "FireflyAbilitySystem|Ability")
	void ApplyAbilityCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool CommitAbilityCost();

	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool CommitAbilityCooldown();

	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Ability", Meta = (BlueprintProtected = "true"))
	bool CommitAbility();

protected:
	UPROPERTY(BlueprintAssignable)
	FAbilityExecutionDelegate OnAbilityCostCommitted;

	UPROPERTY(BlueprintAssignable)
	FAbilityExecutionDelegate OnAbilityCooldownCommitted;

#pragma endregion
	
};

﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbility.h"

#include "FireflyAbilitySystemComponent.h"

UFireflyAbility::UFireflyAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	{
		static FName FuncName = FName(TEXT("ReceiveCanActivateAbility"));
		UFunction* CanActivateFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintCanActivate = ImplementedInBlueprint(CanActivateFunction);
	}
}

UWorld* UFireflyAbility::GetWorld() const
{
	if (AActor* OwnerActor = GetOwnerActor())
	{
		return OwnerActor->GetWorld();
	}

	return nullptr;
}

bool UFireflyAbility::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	if (!IsValid(GetOwnerActor()))
	{
		return false;
	}
		
	UNetDriver* NetDriver = GetOwnerActor()->GetNetDriver();
	if (!NetDriver)
	{
		return false;
	}

	NetDriver->ProcessRemoteFunction(GetOwnerActor(), Function, Parms, OutParms, Stack, this);

	return true;
}

int32 UFireflyAbility::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	return (GetOuter() ? GetOuter()->GetFunctionCallspace(Function, Stack) : FunctionCallspace::Local);
}

AActor* UFireflyAbility::GetOwnerActor() const
{
	if (!IsValid(GetOwnerManager()))
	{
		return nullptr;
	}

	return GetOwnerManager()->GetOwner();
}

ENetRole UFireflyAbility::GetOwnerRole() const
{
	if (!IsValid(GetOwnerActor()))
	{
		return ROLE_None;
	}

	return GetOwnerActor()->GetLocalRole();
}

UFireflyAbilitySystemComponent* UFireflyAbility::GetOwnerManager() const
{
	if (!IsValid(GetOuter()))
	{
		return nullptr;
	}

	return Cast<UFireflyAbilitySystemComponent>(GetOuter());
}

void UFireflyAbility::OnAbilityGranted_Implementation()
{
}

void UFireflyAbility::ActivateAbility()
{
	if (bIsActivating)
	{
		return;
	}

	if (bCancelRequiredAbilities)
	{
		for (auto Ability : GetOwnerManager()->GetActivatingAbilities())
		{
			if (RequiredActivatingAbilities.Contains(Ability->GetClass()))
			{
				Ability->CancelAbility();
			}
		}
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		bCostCommitted = false;
		bCooldownCommitted = false;
	}

	bIsActivating = true;
	ExecuteTagRequirementToOwner(true);	
	OnAbilityActivated.Broadcast();
	ReceiveActivateAbility();
}

void UFireflyAbility::EndAbility()
{
	if (!bIsActivating)
	{
		return;		
	}
	
	bIsActivating = false;
	ExecuteTagRequirementToOwner(false);
	OnAbilityEnded.Broadcast();
	GetOwnerManager()->OnAbilityEndActivation(this);
	ReceiveEndAbility(false);

	if (GetOwnerRole() == ROLE_Authority)
	{
		Client_EndAbility();
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_EndAbility();
	}

	if (bRemoveOnEndedExecution)
	{
		GetOwnerManager()->RemoveAbility(GetClass());
	}
}

void UFireflyAbility::Server_EndAbility_Implementation()
{
	EndAbility();
}

void UFireflyAbility::Client_EndAbility_Implementation()
{
	EndAbility();
}

void UFireflyAbility::CancelAbility()
{
	if (!bIsActivating)
	{
		return;
	}

	bIsActivating = false;
	ExecuteTagRequirementToOwner(false);
	OnAbilityEnded.Broadcast();
	OnAbilityCanceled.Broadcast();
	GetOwnerManager()->OnAbilityEndActivation(this);
	ReceiveEndAbility(true);

	if (GetOwnerRole() == ROLE_Authority)
	{
		Client_CancelAbility();
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_CancelAbility();
	}
}

void UFireflyAbility::Server_CancelAbility_Implementation()
{
	CancelAbility();
}

void UFireflyAbility::Client_CancelAbility_Implementation()
{
	CancelAbility();
}

bool UFireflyAbility::CheckAbilityCost_Implementation() const
{
	return true;
}

void UFireflyAbility::ApplyAbilityCost_Implementation() const
{
	OnAbilityCostCommitted.Broadcast();
}

bool UFireflyAbility::CheckAbilityCooldown_Implementation() const
{
	return true;
}

void UFireflyAbility::ApplyAbilityCooldown_Implementation() const
{
	OnAbilityCooldownCommitted.Broadcast();
}

void UFireflyAbility::CommitAbilityCost()
{
	if (!CheckAbilityCost() || !bIsActivating || GetOwnerRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_CommitAbilityCost();
		return;
	}

	if (GetOwnerRole() != ROLE_Authority || bCostCommitted)
	{
		return;
	}

	bCostCommitted = true;
	ApplyAbilityCost();
}

void UFireflyAbility::Server_CommitAbilityCost_Implementation()
{
	CommitAbilityCost();
}

void UFireflyAbility::CommitAbilityCooldown()
{
	if (!CheckAbilityCooldown() || !bIsActivating || GetOwnerRole() < ROLE_AutonomousProxy)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_CommitAbilityCooldown();
		return;
	}

	if (GetOwnerRole() != ROLE_Authority || bCooldownCommitted)
	{
		return;
	}

	bCooldownCommitted = true;
	ApplyAbilityCooldown();
}

void UFireflyAbility::Server_CommitAbilityCooldown_Implementation()
{
	CommitAbilityCooldown();
}

void UFireflyAbility::CommitAbility()
{
	CommitAbilityCost();
	CommitAbilityCooldown();
}

void UFireflyAbility::SetCooldownTime(float NewCooldownTime)
{
	CooldownTime = NewCooldownTime;
}

void UFireflyAbility::SetCooldownTags(FGameplayTagContainer NewCooldownTags)
{
	CooldownTags = NewCooldownTags;
}

void UFireflyAbility::SetCostSettings(TArray<FFireflyEffectModifierData> NewCostSettings)
{
	CostSettings = NewCostSettings;
}

void UFireflyAbility::ExecuteTagRequirementToOwner(bool bIsActivated)
{
	if (!IsValid(GetOwnerManager()))
	{
		return;
	}

	TArray<FGameplayTag> TagsToUpdate;
	TagsApplyToOwnerOnActivated.GetGameplayTagArray(TagsToUpdate);
	for (auto Tag : TagsToUpdate)
	{
		bIsActivated ? GetOwnerManager()->AddTagToManager(Tag, 1) :
			GetOwnerManager()->RemoveTagFromManager(Tag, 1);
	}
}

bool UFireflyAbility::CanActivateAbility() const
{
	if (bIsActivating || !IsValid(GetOwnerManager()))
	{
		return false;
	}

	FGameplayTagContainer OwnerTags = GetOwnerManager()->GetContainedTags();

	/** Owner的Tag管理器是否包含阻挡该技能激活的Tag */
	if (OwnerTags.HasAnyExact(TagsBlockActivationOnOwnerHas))
	{
		return false;
	}

	/** 是否可执行技能的冷却和消耗 */
	if (!CheckAbilityCooldown() || !CheckAbilityCost())
	{
		return false;
	}

	/** Owner的Tag管理器是否包含该技能激活需要的所有Tag */
	bool bOwnerHasRequiredTags = OwnerTags.HasAll(TagsRequireOwnerHasForActivation);

	/** 蓝图端是否满足技能激活的条件 */
	bool bBlueprintCanActivate = true;
	if (bHasBlueprintCanActivate)
	{
		bBlueprintCanActivate = ReceiveCanActivateAbility();
	}

	/** 技能管理器中是否存在该技能激活需要的前置激活中的技能 */
	bool bHasRequiredActivatingAbility = true;
	if (RequiredActivatingAbilities.Num())
	{
		bHasRequiredActivatingAbility = false;
		for (auto OtherAbility : GetOwnerManager()->GetActivatingAbilities())
		{
			if (RequiredActivatingAbilities.Contains(OtherAbility->GetClass()))
			{
				bHasRequiredActivatingAbility = true;
				break;
			}
		}
	}	

	return bBlueprintCanActivate
		&& bOwnerHasRequiredTags
		&& bHasRequiredActivatingAbility;
}

void UFireflyAbility::OnAbilityInputStarted()
{
	if (!IsValid(GetOwnerManager()->TryActivateAbilityByClass(GetClass())))
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputStarted();
	}
	ReceiveOnAbilityInputStarted();
}

void UFireflyAbility::Server_OnAbilityInputStarted_Implementation()
{
	OnAbilityInputStarted();
}

void UFireflyAbility::OnAbilityInputOngoing()
{
	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputOngoing();
	}
	ReceiveOnAbilityInputOngoing();
}

void UFireflyAbility::Server_OnAbilityInputOngoing_Implementation()
{
	OnAbilityInputOngoing();
}

void UFireflyAbility::OnAbilityInputCanceled()
{
	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputCanceled();
	}
	ReceiveOnAbilityInputCanceled();
}

void UFireflyAbility::Server_OnAbilityInputCanceled_Implementation()
{
	OnAbilityInputCanceled();
}

void UFireflyAbility::OnAbilityInputTriggered()
{
	if (bActivateOnTriggered)
	{
		if (!IsValid(GetOwnerManager()->TryActivateAbilityByClass(GetClass())))
		{
			return;
		}

		if (GetOwnerRole() == ROLE_AutonomousProxy)
		{
			Server_OnAbilityInputTriggered();
		}
		ReceiveOnAbilityInputTriggered();

		return;
	}

	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputTriggered();
	}
	ReceiveOnAbilityInputTriggered();
}

void UFireflyAbility::Server_OnAbilityInputTriggered_Implementation()
{
	OnAbilityInputTriggered();
}

void UFireflyAbility::OnAbilityInputCompleted()
{
	if (!bIsActivating)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_OnAbilityInputCompleted();
	}
	ReceiveOnAbilityInputCompleted();
}

void UFireflyAbility::Server_OnAbilityInputCompleted_Implementation()
{
	OnAbilityInputCompleted();
}
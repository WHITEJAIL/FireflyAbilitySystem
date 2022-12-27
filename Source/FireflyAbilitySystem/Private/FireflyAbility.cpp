// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbility.h"

#include "FireflyAbilitySystemComponent.h"
#include "FireflyEffect.h"

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

bool UFireflyAbility::HasAuthority() const
{
	AActor* Owner = GetOwnerActor();
	check(Owner);

	return Owner->HasAuthority();
}

bool UFireflyAbility::IsLocallyControlled() const
{
	AActor* Owner = GetOwnerActor();
	check(Owner);
	const ENetMode NetMode = Owner->GetNetMode();

	if (NetMode == NM_Standalone)
	{
		// Not networked.
		return true;
	}

	if (NetMode == NM_Client && GetOwnerRole() == ROLE_AutonomousProxy)
	{
		// Networked client in control.
		return true;
	}

	if (Owner->GetRemoteRole() != ROLE_AutonomousProxy && GetOwnerRole() == ROLE_Authority)
	{
		// Local authority in control.
		return true;
	}

	return false;
}

void UFireflyAbility::OnAbilityGranted_Implementation()
{
	if (bActivateOnGranted)
	{
		GetOwnerManager()->TryActivateAbilityByClass(GetClass());
	}
}

void UFireflyAbility::ActivateAbility()
{
	if (bIsActivating || !IsValid(GetOwnerManager()))
	{
		return;
	}

	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();

	if (bCancelRequiredAbilities)
	{
		for (auto Ability : GetOwnerManager()->GetActivatingAbilities())
		{
			if (AbilityClassesRequired.Contains(Ability->GetClass()))
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
	ExecuteAbilityTagRequirementToOwner(true);	
	Manager->OnAbilityActivated.Broadcast(AbilityID, GetClass());
	ReceiveActivateAbility();
}

void UFireflyAbility::EndAbilityInternal()
{
	if (!bIsActivating || !IsValid(GetOwnerManager()))
	{
		return;
	}

	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();

	bIsActivating = false;
	ExecuteAbilityTagRequirementToOwner(false);
	Manager->OnAbilityEnded.Broadcast(AbilityID, GetClass());
	Manager->OnAbilityEndActivation(this);
	ReceiveEndAbility(false);
	
	if (bRemoveOnEndedExecution && GetOwnerRole() == ROLE_Authority)
	{
		GetOwnerManager()->RemoveAbilityByClass(GetClass(), false);
	}

	if (IsValid(MontageToStopOnAbilityEnded) && GetOwnerRole() == ROLE_Authority)
	{
		Multi_StopMontagePlaying(MontageToStopOnAbilityEnded);
		MontageToStopOnAbilityEnded = nullptr;
	}
}

void UFireflyAbility::EndAbility()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Client_EndAbility();
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_EndAbility();
	}
}

void UFireflyAbility::Server_EndAbility_Implementation()
{
	EndAbilityInternal();
}

void UFireflyAbility::Client_EndAbility_Implementation()
{
	EndAbilityInternal();
}

void UFireflyAbility::CancelAbilityInternal()
{
	if (!bIsActivating || !IsValid(GetOwnerManager()))
	{
		return;
	}

	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();

	bIsActivating = false;
	ExecuteAbilityTagRequirementToOwner(false);
	Manager->OnAbilityEnded.Broadcast(AbilityID, GetClass());
	Manager->OnAbilityCanceled.Broadcast(AbilityID, GetClass());
	Manager->OnAbilityEndActivation(this);
	ReceiveEndAbility(true);
}

void UFireflyAbility::CancelAbility()
{
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
	CancelAbilityInternal();
}

void UFireflyAbility::Client_CancelAbility_Implementation()
{
	CancelAbilityInternal();
}

bool UFireflyAbility::CheckAbilityCost_Implementation() const
{
	bool bCostMaySucceed = true;
	for (auto CostSetting : CostSettings)
	{
		if (GetOwnerManager()->CanApplyModifierInstant(CostSetting.AttributeType, CostSetting.ModOperator, CostSetting.ModValue))
		{
			continue;
		}

		bCostMaySucceed = false;
		break;
	}

	return bCostMaySucceed;
}

void UFireflyAbility::ApplyAbilityCost()
{
	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();
	if (!IsValid(Manager))
	{
		return;
	}

	FFireflyEffectDynamicConstructor CostSetup;
	CostSetup.DurationPolicy = EFireflyEffectDurationPolicy::Instant;
	CostSetup.Modifiers = CostSettings;
	GetOwnerManager()->ApplyEffectDynamicConstructorToOwner(nullptr, CostSetup);

	GetOwnerManager()->OnAbilityCostCommitted.Broadcast(AbilityID, GetClass());
}

bool UFireflyAbility::CheckAbilityCooldown_Implementation() const
{
	if (!CooldownTags.IsValid())
	{
		return true;
	}

	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();
	if (!IsValid(Manager))
	{
		return false;
	}

	return Manager->GetActiveEffectsByTag(CooldownTags).Num() <= 0;
}

void UFireflyAbility::ApplyAbilityCooldown()
{
	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();
	if (!IsValid(Manager))
	{
		return;
	}

	FFireflyEffectDynamicConstructor CostSetup;
	CostSetup.DurationPolicy = EFireflyEffectDurationPolicy::HasDuration;
	CostSetup.Duration = CooldownTime;
	CostSetup.TagsForEffectAsset = CooldownTags;
	GetOwnerManager()->ApplyEffectDynamicConstructorToOwner(nullptr, CostSetup);

	GetOwnerManager()->OnAbilityCooldownCommitted.Broadcast(AbilityID, GetClass(), CooldownTime);
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

float UFireflyAbility::CalculateCooldownTime_Implementation()
{
	return CooldownTime;
}

void UFireflyAbility::SetCooldownTime(float NewCooldownTime)
{
	CooldownTime = NewCooldownTime;
}

void UFireflyAbility::SetCooldownTags(FGameplayTagContainer NewCooldownTags)
{
	CooldownTags = NewCooldownTags;
}

TArray<FFireflyEffectModifierData> UFireflyAbility::CalculateCostSettings_Implementation()
{
	return CostSettings;
}

void UFireflyAbility::SetCostSettings(TArray<FFireflyEffectModifierData> NewCostSettings)
{
	CostSettings = NewCostSettings;
}

void UFireflyAbility::ExecuteAbilityTagRequirementToOwner(bool bIsActivated)
{
	if (!IsValid(GetOwnerManager()))
	{
		return;
	}

	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();

	Manager->UpdateBlockAndCancelAbilityTags(TagsOfAbilitiesWillBeBlocked, TagsOfAbilitiesWillBeCanceled, bIsActivated);
	if (bIsActivated)
	{
		Manager->AddTagsToManager(TagsApplyToOwnerOnActivated, 1);
	}
	else
	{
		Manager->RemoveTagsFromManager(TagsApplyToOwnerOnActivated, 1);
	}
}

bool UFireflyAbility::CanActivateAbility() const
{
	if (bIsActivating || !IsValid(GetOwnerManager()))
	{
		return false;
	}

	UFireflyAbilitySystemComponent* Manager = GetOwnerManager();

	FGameplayTagContainer OwnerTags = Manager->GetContainedTags();

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
	if (AbilityClassesRequired.Num())
	{
		bHasRequiredActivatingAbility = false;
		for (auto OtherAbility : Manager->GetActivatingAbilities())
		{
			if (AbilityClassesRequired.Contains(OtherAbility->GetClass()))
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

void UFireflyAbility::ActivateAbilityByMessage(const FFireflyMessageEventData EventData)
{
	ReceiveActivateAbilityByMessage(EventData);
}

void UFireflyAbility::OnAbilityInputStarted()
{
	if (!GetOwnerManager()->TryActivateAbilityByClass(GetClass()))
	{
		return;
	}

	Server_OnAbilityInputStarted();
	OnAbilityInputStartedInternal();
}

void UFireflyAbility::OnAbilityInputStartedInternal()
{
	ReceiveOnAbilityInputStarted();
}

void UFireflyAbility::Server_OnAbilityInputStarted_Implementation()
{
	OnAbilityInputStartedInternal();
}

void UFireflyAbility::OnAbilityInputOngoing()
{
	if (!bIsActivating)
	{
		return;
	}

	Server_OnAbilityInputOngoing();
	OnAbilityInputOngoingInternal();
}

void UFireflyAbility::OnAbilityInputOngoingInternal()
{
	ReceiveOnAbilityInputOngoing();
}

void UFireflyAbility::Server_OnAbilityInputOngoing_Implementation()
{
	OnAbilityInputOngoingInternal();
}

void UFireflyAbility::OnAbilityInputCanceled()
{
	if (!bIsActivating)
	{
		return;
	}

	Server_OnAbilityInputCanceled();
	OnAbilityInputCanceledInternal();
}

void UFireflyAbility::OnAbilityInputCanceledInternal()
{
	ReceiveOnAbilityInputCanceled();
}

void UFireflyAbility::Server_OnAbilityInputCanceled_Implementation()
{
	OnAbilityInputCanceledInternal();
}

void UFireflyAbility::OnAbilityInputTriggered()
{
	if (bActivateOnTriggered)
	{
		if (!GetOwnerManager()->TryActivateAbilityByClass(GetClass()))
		{
			return;
		}

		Server_OnAbilityInputTriggered();
		OnAbilityInputTriggeredInternal();

		return;
	}

	if (!bIsActivating)
	{
		return;
	}

	Server_OnAbilityInputTriggered();
	OnAbilityInputTriggeredInternal();
}

void UFireflyAbility::OnAbilityInputTriggeredInternal()
{
	ReceiveOnAbilityInputTriggered();
}

void UFireflyAbility::Server_OnAbilityInputTriggered_Implementation()
{
	OnAbilityInputTriggeredInternal();
}

void UFireflyAbility::OnAbilityInputCompleted()
{
	if (!bIsActivating)
	{
		return;
	}

	Server_OnAbilityInputCompleted();
	OnAbilityInputCompletedInternal();
}

void UFireflyAbility::OnAbilityInputCompletedInternal()
{
	ReceiveOnAbilityInputCompleted();
}

void UFireflyAbility::Server_OnAbilityInputCompleted_Implementation()
{
	OnAbilityInputCompletedInternal();
}

UAnimInstance* UFireflyAbility::GetAnimInstanceOfOwner() const
{
	if (!IsValid(GetOwnerActor()))
	{
		return nullptr;
	}

	USkeletalMeshComponent* OwnerSkeletalMesh = nullptr;
	if (!IsValid(GetOwnerActor()->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
	{
		return nullptr;
	}

	OwnerSkeletalMesh = Cast<USkeletalMeshComponent>(GetOwnerActor()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));

	return OwnerSkeletalMesh->GetAnimInstance();
}

void UFireflyAbility::PlayMontageForOwnerInternal(UAnimMontage* MontageToPlay, float PlayRate, FName Section)
{
	UAnimInstance* OwnerAnimInstance = GetAnimInstanceOfOwner();
	if (!IsValid(OwnerAnimInstance))
	{
		return;
	}

	if (OwnerAnimInstance->Montage_IsPlaying(MontageToPlay)
		&& OwnerAnimInstance->Montage_GetCurrentSection(MontageToPlay) == Section)
	{
		return;
	}

	OwnerAnimInstance->Montage_Play(MontageToPlay, PlayRate, EMontagePlayReturnType::Duration);
	OwnerAnimInstance->Montage_JumpToSection(Section, MontageToPlay);

	OnMontageEnded.BindUObject(this, &UFireflyAbility::OnOwnerMontageEnded);
	OwnerAnimInstance->Montage_SetEndDelegate(OnMontageEnded, MontageToPlay);
	OnMontageBlendOut.BindUObject(this, &UFireflyAbility::OnOwnerMontageBlendOut);
	OwnerAnimInstance->Montage_SetBlendingOutDelegate(OnMontageBlendOut, MontageToPlay);
}

float UFireflyAbility::PlayMontageForOwner(UAnimMontage* MontageToPlay, float PlayRate, FName Section, bool bStopOnAbilityEnded)
{
	if (!bIsActivating || !IsValid(MontageToPlay))
	{
		return 0.f;
	}

	Server_PlayMontageForOwner(MontageToPlay, PlayRate, Section, bStopOnAbilityEnded);

	return MontageToPlay->GetPlayLength() / (PlayRate * MontageToPlay->RateScale);
}

void UFireflyAbility::Server_PlayMontageForOwner_Implementation(UAnimMontage* MontageToPlay, float PlayRate,
	FName Section, bool bStopOnAbilityEnded)
{
	if (bStopOnAbilityEnded)
	{
		MontageToStopOnAbilityEnded = MontageToPlay;
	}

	Multi_PlayMontageForOwner(MontageToPlay, PlayRate, Section);
}

void UFireflyAbility::Multi_PlayMontageForOwner_Implementation(UAnimMontage* MontageToPlay, float PlayRate,
	FName Section)
{
	PlayMontageForOwnerInternal(MontageToPlay, PlayRate, Section);
}

void UFireflyAbility::OnOwnerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ReceiveOnOwnerMontageEnded(Montage, bInterrupted);
	OnMontageEnded.Unbind();
}

void UFireflyAbility::OnOwnerMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	ReceiveOnOwnerMontageBlendOut(Montage, bInterrupted);
	OnMontageBlendOut.Unbind();
}

void UFireflyAbility::Multi_StopMontagePlaying_Implementation(UAnimMontage* MontageToStop)
{
	UAnimInstance* OwnerAnimInstance = GetAnimInstanceOfOwner();
	if (!IsValid(OwnerAnimInstance))
	{
		return;
	}

	OwnerAnimInstance->Montage_Stop(0.1f, MontageToStop);
}

UFireflyEffect* UFireflyAbility::MakeDynamicEffectByID(FName EffectID) const
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->MakeDynamicEffectByID(EffectID);
}

UFireflyEffect* UFireflyAbility::MakeDynamicEffectByClass(TSubclassOf<UFireflyEffect> EffectType, FName EffectID) const
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->MakeDynamicEffectByClass(EffectType, EffectID);
}

UFireflyEffect* UFireflyAbility::AssignDynamicEffectAssetTags(UFireflyEffect* EffectInstance,
	FGameplayTagContainer NewEffectAssetTags)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->AssignDynamicEffectAssetTags(EffectInstance, NewEffectAssetTags);
}

UFireflyEffect* UFireflyAbility::AssignDynamicEffectGrantTags(UFireflyEffect* EffectInstance,
	FGameplayTagContainer NewEffectGrantTags)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->AssignDynamicEffectGrantTags(EffectInstance, NewEffectGrantTags);
}

UFireflyEffect* UFireflyAbility::SetDynamicEffectDuration(UFireflyEffect* EffectInstance, float Duration)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->SetDynamicEffectDuration(EffectInstance, Duration);
}

UFireflyEffect* UFireflyAbility::SetDynamicEffectPeriodicInterval(UFireflyEffect* EffectInstance,
	float PeriodicInterval)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->SetDynamicEffectPeriodicInterval(EffectInstance, PeriodicInterval);
}

UFireflyEffect* UFireflyAbility::SetDynamicEffectModifierValue(UFireflyEffect* EffectInstance,
	EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, float ModValue)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->SetDynamicEffectModifierValue(EffectInstance, AttributeType, ModOperator, ModValue);
}

UFireflyEffect* UFireflyAbility::AssignDynamicEffectModifier(UFireflyEffect* EffectInstance,
	FFireflyEffectModifierData NewModifier)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->AssignDynamicEffectModifier(EffectInstance, NewModifier);
}

UFireflyEffect* UFireflyAbility::AssignDynamicEffectSpecificProperty(UFireflyEffect* EffectInstance,
	FFireflySpecificProperty NewSpecificProperty)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return nullptr;
	}

	return FireflyAbilitySystem->AssignDynamicEffectSpecificProperty(EffectInstance, NewSpecificProperty);
}

void UFireflyAbility::ApplyEffectToOwner(UFireflyEffect* EffectInstance, int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectToOwner(GetOwnerActor(), EffectInstance, StackToApply);
}

void UFireflyAbility::ApplyEffectToTarget(AActor* Target, UFireflyEffect* EffectInstance, int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectToTarget(Target, EffectInstance, StackToApply);
}

void UFireflyAbility::ApplyEffectToOwnerByID(FName EffectID, int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectToOwnerByID(GetOwnerActor(), EffectID, StackToApply);
}

void UFireflyAbility::ApplyEffectToTargetByID(AActor* Target, FName EffectID, int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectToTargetByID(Target, EffectID, StackToApply);
}

void UFireflyAbility::ApplyEffectToOwnerByClass(TSubclassOf<UFireflyEffect> EffectType, FName EffectID,
	int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectToOwnerByClass(GetOwnerActor(), EffectType, EffectID, StackToApply);
}

void UFireflyAbility::ApplyEffectToTargetByClass(AActor* Target, TSubclassOf<UFireflyEffect> EffectType, FName EffectID,
	int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectToTargetByClass(Target, EffectType, EffectID, StackToApply);
}

void UFireflyAbility::ApplyEffectDynamicConstructorToOwner(FFireflyEffectDynamicConstructor EffectSetup,
	int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectDynamicConstructorToOwner(GetOwnerActor(), EffectSetup, StackToApply);
}

void UFireflyAbility::ApplyEffectDynamicConstructorToTarget(AActor* Target,
	FFireflyEffectDynamicConstructor EffectSetup, int32 StackToApply)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->ApplyEffectDynamicConstructorToTarget(Target, EffectSetup, StackToApply);
}

void UFireflyAbility::RemoveActiveEffectsByID(FName EffectID, int32 StackToRemove)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->RemoveActiveEffectsByID(EffectID, StackToRemove);
}

void UFireflyAbility::RemoveActiveEffectsByClass(TSubclassOf<UFireflyEffect> EffectType, int32 StackToRemove)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->RemoveActiveEffectsByClass(EffectType, StackToRemove);
}

void UFireflyAbility::RemoveActiveEffectsWithTags(FGameplayTagContainer RemoveTags)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->RemoveActiveEffectsWithTags(RemoveTags);
}

void UFireflyAbility::RemoveSingleActiveEffectByID(FName EffectID, int32 StackToRemove)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->RemoveSingleActiveEffectByID(EffectID, StackToRemove);
}

void UFireflyAbility::RemoveSingleActiveEffectByClass(TSubclassOf<UFireflyEffect> EffectType, int32 StackToRemove)
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return;
	}

	FireflyAbilitySystem->RemoveSingleActiveEffectByClass(EffectType, StackToRemove);
}

float UFireflyAbility::GetOwnerAttributeValue(EFireflyAttributeType AttributeType) const
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return 0.f;
	}

	return FireflyAbilitySystem->GetAttributeValue(AttributeType);
}

float UFireflyAbility::GetOwnerAttributeBaseValue(EFireflyAttributeType AttributeType) const
{
	UFireflyAbilitySystemComponent* FireflyAbilitySystem = GetOwnerManager();
	if (!IsValid(FireflyAbilitySystem))
	{
		return 0.f;
	}

	return FireflyAbilitySystem->GetAttributeBaseValue(AttributeType);
}

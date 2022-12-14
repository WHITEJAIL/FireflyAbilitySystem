// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbilitySystemComponent.h"

#include "EnhancedInputComponent.h"
#include "FireflyAbilitySystemLibrary.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UFireflyAbilitySystemComponent::UFireflyAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UFireflyAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UFireflyAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UFireflyAbilitySystemComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (UFireflyAbility* Ability : GrantedAbilities)
	{
		if (IsValid(Ability))
		{
			bWroteSomething |= Channel->ReplicateSubobject(Ability, *Bunch, *RepFlags);
		}
	}

	for (UFireflyEffect* Effect : ActiveEffects)
	{
		if (IsValid(Effect))
		{
			bWroteSomething |= Channel->ReplicateSubobject(Effect, *Bunch, *RepFlags);
		}
	}

	for (UFireflyAttribute* Attribute : AttributeContainer)
	{
		if (IsValid(Attribute))
		{
			bWroteSomething |= Channel->ReplicateSubobject(Attribute, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void UFireflyAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFireflyAbilitySystemComponent, GrantedAbilities);
	DOREPLIFETIME(UFireflyAbilitySystemComponent, ActiveEffects);
	DOREPLIFETIME(UFireflyAbilitySystemComponent, AttributeContainer);
}

bool UFireflyAbilitySystemComponent::HasAuthority() const
{
	AActor* Owner = GetOwner();
	check(Owner);

	return Owner->HasAuthority();
}

bool UFireflyAbilitySystemComponent::IsLocallyControlled() const
{
	check(GetOwner());
	const ENetMode NetMode = GetOwner()->GetNetMode();

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

	if (GetOwner()->GetRemoteRole() != ROLE_AutonomousProxy && GetOwnerRole() == ROLE_Authority)
	{
		// Local authority in control.
		return true;
	}

	return false;
}

UFireflyAbility* UFireflyAbilitySystemComponent::GetGrantedAbilityByID(FName AbilityID) const
{
	if (AbilityID == NAME_None)
	{
		return nullptr;
	}

	UFireflyAbility* OutAbility = nullptr;
	for (auto Ability : GrantedAbilities)
	{
		if (Ability->AbilityID == AbilityID)
		{
			OutAbility = Ability;
			break;
		}
	}

	return OutAbility;
}

TArray<UFireflyAbility*> UFireflyAbilitySystemComponent::GetGrantedAbilityByTag(FGameplayTag AbilityTag) const
{
	TArray<UFireflyAbility*> OutAbilities = TArray<UFireflyAbility*>{};
	for (auto Ability : GrantedAbilities)
	{
		if (Ability->TagsForAbilityAsset.HasTagExact(AbilityTag))
		{
			OutAbilities.Add(Ability);
		}
	}

	return OutAbilities;
}

UFireflyAbility* UFireflyAbilitySystemComponent::GetGrantedAbilityByClass(
	TSubclassOf<UFireflyAbility> AbilityType) const
{
	UFireflyAbility* OutAbility = nullptr;
	for (auto Ability : GrantedAbilities)
	{
		if (Ability->GetClass() == AbilityType)
		{
			OutAbility = Ability;
			break;
		}
	}

	return OutAbility;
}

bool UFireflyAbilitySystemComponent::GrantAbilityByID(FName AbilityID)
{
	if (!HasAuthority() || AbilityID == NAME_None)
	{
		return false;
	}

	TSubclassOf<UFireflyAbility> AbilityToGrant = UFireflyAbilitySystemLibrary::GetAbilityClassFromCache(AbilityID);
	if (!IsValid(AbilityToGrant))
	{
		return false;
	}

	if (IsValid(GetGrantedAbilityByClass(AbilityToGrant)))
	{
		return false;
	}

	UFireflyAbility* NewAbility = NewObject<UFireflyAbility>(this, AbilityToGrant);
	NewAbility->AbilityID = AbilityID;
	GrantedAbilities.Emplace(NewAbility);
	NewAbility->OnAbilityGranted();

	return true;
}

bool UFireflyAbilitySystemComponent::GrantAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToGrant, FName AbilityID)
{
	if (!IsValid(AbilityToGrant) || !HasAuthority())
	{
		return false;
	}

	if (IsValid(GetGrantedAbilityByClass(AbilityToGrant)))
	{
		return false;
	}

	UFireflyAbility* NewAbility = NewObject<UFireflyAbility>(this, AbilityToGrant);
	NewAbility->AbilityID = AbilityID;
	GrantedAbilities.Emplace(NewAbility);
	NewAbility->OnAbilityGranted();

	return true;
}

void UFireflyAbilitySystemComponent::RemoveAbilityByID(FName AbilityID, bool bRemoveOnEnded)
{
	if (!HasAuthority() || AbilityID == NAME_None)
	{
		return;
	}

	UFireflyAbility* Ability = GetGrantedAbilityByID(AbilityID);
	if (!IsValid(Ability))
	{
		return;
	}

	if (Ability->bIsActivating)
	{
		if (bRemoveOnEnded)
		{
			Ability->bRemoveOnEndedExecution = true;

			return;
		}

		Ability->CancelAbility();
	}
	GrantedAbilities.RemoveSingle(Ability);
	Ability->MarkAsGarbage();
}

void UFireflyAbilitySystemComponent::RemoveAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToRemove, bool bRemoveOnEnded)
{
	if (!IsValid(AbilityToRemove) || !HasAuthority())
	{
		return;
	}

	UFireflyAbility* Ability = GetGrantedAbilityByClass(AbilityToRemove);
	if (!IsValid(Ability))
	{
		return;
	}

	if (Ability->bIsActivating)
	{
		if (bRemoveOnEnded)
		{
			Ability->bRemoveOnEndedExecution = true;

			return;
		}

		Ability->CancelAbility();		
	}
	GrantedAbilities.RemoveSingle(Ability);
	Ability->MarkAsGarbage();
}

void UFireflyAbilitySystemComponent::TryActivateAbilityInternal(UFireflyAbility* Ability)
{
	if (!IsValid(Ability))
	{
		return;
	}

	Server_TryActivateAbility(Ability);
}

void UFireflyAbilitySystemComponent::ActivateAbilityInternal(UFireflyAbility* Ability)
{
	if (!IsValid(Ability))
	{
		return;
	}

	if (ActivatingAbilities.Contains(Ability))
	{
		return;
	}

	Ability->ActivateAbility();
	ActivatingAbilities.Emplace(Ability);
}

void UFireflyAbilitySystemComponent::Server_TryActivateAbility_Implementation(UFireflyAbility* Ability)
{
	if (!IsValid(Ability))
	{
		return;
	}

	if (!Ability->CanActivateAbility())
	{
		return;
	}

	if (Ability->TagsForAbilityAsset.HasAnyExact(GetBlockAbilityTags()))
	{
		return;
	}

	ActivateAbilityInternal(Ability);
	Client_ActivateAbility(Ability);
}

void UFireflyAbilitySystemComponent::Client_ActivateAbility_Implementation(UFireflyAbility* Ability)
{
	if (!IsValid(Ability))
	{
		return;
	}

	ActivateAbilityInternal(Ability);
}

bool UFireflyAbilitySystemComponent::TryActivateAbilityByID(FName AbilityID)
{
	if (AbilityID == NAME_None)
	{
		return false;
	}

	UFireflyAbility* Ability = GetGrantedAbilityByID(AbilityID);
	if (!IsValid(Ability))
	{
		return false;
	}

	TryActivateAbilityInternal(Ability);

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		return Ability->CanActivateAbility()
			&& Ability->TagsForAbilityAsset.HasAnyExact(
				GetBlockAbilityTags());
	}

	return Ability->bIsActivating;
}

bool UFireflyAbilitySystemComponent::TryActivateAbilityByClass(
	TSubclassOf<UFireflyAbility> AbilityToActivate)
{
	if (!IsValid(AbilityToActivate))
	{
		return false;
	}

	UFireflyAbility* Ability = GetGrantedAbilityByClass(AbilityToActivate);
	if (!IsValid(Ability))
	{
		return false;
	}

	TryActivateAbilityInternal(Ability);

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		return Ability->CanActivateAbility()
			&& Ability->TagsForAbilityAsset.HasAnyExact(
				GetBlockAbilityTags());
	}

	return Ability->bIsActivating;
}

void UFireflyAbilitySystemComponent::CancelAbilityByID(FName AbilityID)
{
	UFireflyAbility* AbilityToCancel = GetGrantedAbilityByID(AbilityID);
	if (!IsValid(AbilityToCancel))
	{
		return;
	}

	AbilityToCancel->CancelAbility();
}

void UFireflyAbilitySystemComponent::CancelAbilityByClass(TSubclassOf<UFireflyAbility> AbilityType)
{
	UFireflyAbility* AbilityToCancel = GetGrantedAbilityByClass(AbilityType);
	if (!IsValid(AbilityToCancel))
	{
		return;
	}

	AbilityToCancel->CancelAbility();
}

void UFireflyAbilitySystemComponent::CancelAbilitiesWithTags(FGameplayTagContainer CancelTags)
{
	if (!HasAuthority())
	{
		return;
	}

	for (auto Ability : GetActivatingAbilities())
	{
		if (Ability->TagsForAbilityAsset.HasAnyExact(CancelTags))
		{
			Ability->CancelAbility();
		}
	}
}

void UFireflyAbilitySystemComponent::OnAbilityEndActivation(UFireflyAbility* AbilityJustEnded)
{
	ActivatingAbilities.RemoveSingle(AbilityJustEnded);
}

void UFireflyAbilitySystemComponent::SetAbilityCooldownRemaining(TSubclassOf<UFireflyAbility> AbilityType,
	float NewTimeRemaining)
{
	if (!IsValid(AbilityType) || !HasAuthority())
	{
		return;
	}

	UFireflyAbility* Ability =  GetGrantedAbilityByClass(AbilityType);
	if (!IsValid(Ability))
	{
		return;
	}

	TArray<UFireflyEffect*> CooldownEffects = GetActiveEffectsByTag(Ability->CooldownTags);
	if (!CooldownEffects.IsValidIndex(0))
	{
		return;
	}

	CooldownEffects[0]->SetTimeRemainingOfDuration(NewTimeRemaining);

	OnAbilityCooldownRemainingChanged.Broadcast(Ability->AbilityID, AbilityType, NewTimeRemaining, CooldownEffects[0]->Duration);
}

FGameplayTagContainer UFireflyAbilitySystemComponent::GetBlockAbilityTags() const
{
	FGameplayTagContainer OutTags;
	for (auto TagCount : BlockAbilityTags)
	{
		OutTags.AddTag(TagCount.Key);
	}

	return OutTags;
}

void UFireflyAbilitySystemComponent::UpdateBlockAndCancelAbilityTags(FGameplayTagContainer BlockTags,
                                                                     FGameplayTagContainer CancelTags, bool bIsActivated)
{
	if (bIsActivated)
	{
		CancelAbilitiesWithTags(CancelTags);

		TArray<FGameplayTag> Tags;
		BlockTags.GetGameplayTagArray(Tags);
		for (auto TagToAdd : Tags)
		{
			int32& Count = BlockAbilityTags.FindOrAdd(TagToAdd);
			++Count;
		}
	}
	else
	{
		TArray<FGameplayTag> Tags;
		BlockTags.GetGameplayTagArray(Tags);
		for (auto TagToRemove : Tags)
		{
			if (!BlockAbilityTags.Contains(TagToRemove))
			{
				continue;
			}

			int32* CountToMinus = BlockAbilityTags.Find(TagToRemove);
			*CountToMinus = FMath::Clamp<int32>(*CountToMinus - 1, 0, *CountToMinus);

			if (*CountToMinus == 0)
			{
				BlockAbilityTags.Remove(TagToRemove);
			}
		}
	}
}

UEnhancedInputComponent* UFireflyAbilitySystemComponent::GetEnhancedInputComponentFromOwner() const
{
	if (!IsValid(GetOwner()))
	{
		return nullptr;
	}

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		return nullptr;
	}

	if (!IsValid(PawnOwner->InputComponent))
	{
		return nullptr;
	}

	return Cast<UEnhancedInputComponent>(PawnOwner->InputComponent);
}

void UFireflyAbilitySystemComponent::BindAbilityToInput(TSubclassOf<UFireflyAbility> AbilityToBind,
	UInputAction* InputToBind)
{
	if (!IsValid(InputToBind) || !IsLocallyControlled())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInput = GetEnhancedInputComponentFromOwner();
	if (!IsValid(EnhancedInput))
	{
		return;
	}

	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(InputToBind);
	if (!AbilitiesBoundToInput)
	{
		AbilitiesBoundToInput = new FFireflyAbilitiesBoundToInput();
		AbilitiesInputBound.Add(InputToBind, *AbilitiesBoundToInput);

		AbilitiesBoundToInput->HandleStarted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Started, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionStarted, InputToBind).GetHandle();
		AbilitiesBoundToInput->HandleOngoing = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Ongoing, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionOngoing, InputToBind).GetHandle();
		AbilitiesBoundToInput->HandleCanceled = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Canceled, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionCanceled, InputToBind).GetHandle();
		AbilitiesBoundToInput->HandleTriggered = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Triggered, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionTriggered, InputToBind).GetHandle();
		AbilitiesBoundToInput->HandleCompleted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Completed, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionCompleted, InputToBind).GetHandle();
	}

	if (IsValid(AbilityToBind))
	{
		AbilitiesBoundToInput->Abilities.AddUnique(AbilityToBind);
	}	
}

void UFireflyAbilitySystemComponent::UnbindAbilityWithInput(TSubclassOf<UFireflyAbility> AbilityToUnbind,
	UInputAction* InputToUnbind)
{
	if (!IsValid(InputToUnbind) || !IsValid(AbilityToUnbind))
	{
		return;
	}

	if (!IsLocallyControlled())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInput = GetEnhancedInputComponentFromOwner();
	if (!IsValid(EnhancedInput))
	{
		return;
	}

	if (!IsValid(GetGrantedAbilityByClass(AbilityToUnbind)))
	{
		return;
	}

	UFireflyAbility* Ability = Cast<UFireflyAbility>(GetGrantedAbilityByClass(AbilityToUnbind));
	if (!IsValid(Ability))
	{
		return;
	}

	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(InputToUnbind);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	if (!AbilitiesBoundToInput->Abilities.Contains(AbilityToUnbind))
	{
		return;
	}

	AbilitiesBoundToInput->Abilities.RemoveSingleSwap(AbilityToUnbind);
}

void UFireflyAbilitySystemComponent::OnAbilityInputActionStarted(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetGrantedAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility* Ability = Cast<UFireflyAbility>(GetGrantedAbilityByClass(AbilityClass));
		if (!Ability->CanActivateAbility())
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputStarted();
	}
}

void UFireflyAbilitySystemComponent::OnAbilityInputActionOngoing(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetGrantedAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility* Ability = Cast<UFireflyAbility>(GetGrantedAbilityByClass(AbilityClass));
		if (!Ability->bIsActivating)
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputOngoing();
	}
}

void UFireflyAbilitySystemComponent::OnAbilityInputActionCanceled(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetGrantedAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility* Ability = Cast<UFireflyAbility>(GetGrantedAbilityByClass(AbilityClass));
		if (!Ability->bIsActivating)
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputCanceled();
	}
}

void UFireflyAbilitySystemComponent::OnAbilityInputActionTriggered(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetGrantedAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility* Ability = Cast<UFireflyAbility>(GetGrantedAbilityByClass(AbilityClass));
		if (Ability->bActivateOnTriggered && !Ability->CanActivateAbility())
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputTriggered();
	}
}

void UFireflyAbilitySystemComponent::OnAbilityInputActionCompleted(UInputAction* Input)
{
	FFireflyAbilitiesBoundToInput* AbilitiesBoundToInput = AbilitiesInputBound.Find(Input);
	if (AbilitiesBoundToInput == nullptr)
	{
		return;
	}

	TArray<UFireflyAbility*> Abilities;
	for (auto AbilityClass : AbilitiesBoundToInput->Abilities)
	{
		if (!IsValid(GetGrantedAbilityByClass(AbilityClass)))
		{
			continue;
		}

		UFireflyAbility* Ability = Cast<UFireflyAbility>(GetGrantedAbilityByClass(AbilityClass));
		if (!Ability->bIsActivating)
		{
			continue;
		}

		Abilities.Emplace(Ability);
	}

	for (auto Ability : Abilities)
	{
		Ability->OnAbilityInputCompleted();
	}
}

UFireflyAttribute* UFireflyAbilitySystemComponent::GetAttributeByType(EFireflyAttributeType AttributeType) const
{
	UFireflyAttribute* OutAttribute = nullptr;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->AttributeType == AttributeType)
		{
			OutAttribute = Attribute;
			break;
		}
	}

	return OutAttribute;
}

UFireflyAttribute* UFireflyAbilitySystemComponent::GetAttributeByName(FName AttributeName) const
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFireflyAttributeType"), true);
	if (!EnumPtr)
	{
		return nullptr;
	}

	UFireflyAttribute* OutAttribute = nullptr;
	for (auto Attribute : AttributeContainer)
	{
		if (EnumPtr->GetDisplayNameTextByValue(Attribute->AttributeType).ToString() == AttributeName.ToString())
		{
			OutAttribute = Attribute;
			break;
		}
	}

	return OutAttribute;
}

float UFireflyAbilitySystemComponent::GetAttributeValue(EFireflyAttributeType AttributeType) const
{
	for (const auto Attribute : AttributeContainer)
	{
		if (Attribute->AttributeType == AttributeType)
		{
			return Attribute->GetCurrentValue();
		}
	}

	return 0.f;
}

float UFireflyAbilitySystemComponent::GetAttributeBaseValue(EFireflyAttributeType AttributeType) const
{
	float OutBaseValue = 0.f;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->AttributeType == AttributeType)
		{
			OutBaseValue = Attribute->GetBaseValueToUse();
			break;
		}
	}

	return OutBaseValue;
}

void UFireflyAbilitySystemComponent::ConstructAttributeByConstructor(FFireflyAttributeConstructor AttributeConstructor)
{
	if (!HasAuthority())
	{
		return;
	}

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this);
	if (!IsValid(NewAttribute))
	{
		return;
	}
	NewAttribute->AttributeType = AttributeConstructor.AttributeType;
	NewAttribute->bAttributeHasRange = AttributeConstructor.bAttributeHasRange;
	NewAttribute->RangeMinValue = AttributeConstructor.RangeMinValue;
	NewAttribute->RangeMaxValue = AttributeConstructor.RangeMaxValue;
	NewAttribute->RangeMaxValueType = AttributeConstructor.RangeMaxValueType;
	NewAttribute->InitAttributeInstance();

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::ConstructAttributeByClass(TSubclassOf<UFireflyAttribute> AttributeClass)
{
	if (!IsValid(AttributeClass) || !HasAuthority())
	{
		return;
	}

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, AttributeClass);
	if (!IsValid(NewAttribute))
	{
		return;
	}NewAttribute->InitAttributeInstance();

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::ConstructAttributeByType(EFireflyAttributeType AttributeType)
{
	if (!HasAuthority())
	{
		return;
	}

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this);
	if (!IsValid(NewAttribute))
	{
		return;
	}
	NewAttribute->AttributeType = AttributeType;
	NewAttribute->InitAttributeInstance();

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::InitializeAttributeByType(EFireflyAttributeType AttributeType, float NewInitValue)
{
	if (!HasAuthority())
	{
		return;
	}

	UFireflyAttribute* AttributeToInit = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToInit))
	{
		return;
	}

	AttributeToInit->InitializeAttributeValue(NewInitValue);
}

void UFireflyAbilitySystemComponent::InitializeAttributeByName(FName AttributeName, float NewInitValue)
{
	if (!HasAuthority())
	{
		return;
	}

	UFireflyAttribute* AttributeToInit = GetAttributeByName(AttributeName);
	if (!IsValid(AttributeToInit))
	{
		return;
	}

	AttributeToInit->InitializeAttributeValue(NewInitValue);
}

void UFireflyAbilitySystemComponent::PreModiferApplied(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply)
{
}

void UFireflyAbilitySystemComponent::PostModiferApplied(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply)
{
}

#define FIREFLY_ATTRIBUTE_MODIFIER_APPLY(ModOperatorName) \
	{ \
		if (!AttributeToMod->##ModOperatorName##Mods.Contains(FFireflyAttributeModifier(ModSource, ModValue))) \
		{ \
			AttributeToMod->##ModOperatorName##Mods.Push(FFireflyAttributeModifier(ModSource, ModValue, StackToApply)); \
			break; \
		} \
		\
		for (FFireflyAttributeModifier& Modifier : AttributeToMod->##ModOperatorName##Mods) \
		{ \
			if (Modifier.ModSource == ModSource) \
			{ \
				Modifier.StackCount = StackToApply; \
				break; \
			} \
		} \
		break; \
	}

void UFireflyAbilitySystemComponent::ApplyModifierToAttribute(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply)
{
	if (!HasAuthority() || !IsValid(ModSource))
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
		{
			break;
		}
	case EFireflyAttributeModOperator::Plus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(Plus);
		}
	case EFireflyAttributeModOperator::Minus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(Minus);
		}
	case EFireflyAttributeModOperator::Multiply:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(Multiply);
		}
	case EFireflyAttributeModOperator::Divide:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(Divide);
		}
	case EFireflyAttributeModOperator::InnerOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(InnerOverride);
		}
	case EFireflyAttributeModOperator::OuterOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(OuterOverride);
		}
	}

	PreModiferApplied(AttributeType, ModOperator, ModSource, ModValue, StackToApply);

	AttributeToMod->UpdateCurrentValue();

	PostModiferApplied(AttributeType, ModOperator, ModSource, ModValue, StackToApply);
}

#define FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(ModOperatorName) \
	{ \
		if (AttributeToMod->##ModOperatorName##Mods.Contains(ModifierToRemove)) \
		{ \
			AttributeToMod->##ModOperatorName##Mods.RemoveSingle(ModifierToRemove); \
		} \
		break; \
	}

void UFireflyAbilitySystemComponent::RemoveModifierFromAttribute(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue)
{
	if (!HasAuthority() || !IsValid(ModSource))
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	FFireflyAttributeModifier ModifierToRemove = FFireflyAttributeModifier(ModSource, ModValue);

	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
		{
			break;
		}
	case EFireflyAttributeModOperator::Plus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Plus);
		}
	case EFireflyAttributeModOperator::Minus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Minus);
		}
	case EFireflyAttributeModOperator::Multiply:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Multiply);
		}
	case EFireflyAttributeModOperator::Divide:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Divide);
		}
	case EFireflyAttributeModOperator::InnerOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(InnerOverride);
	}
	case EFireflyAttributeModOperator::OuterOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(OuterOverride);
		}
	}

	AttributeToMod->UpdateCurrentValue();
}

#define FIREFLY_ATTRIBUTE_MODIFIER_ACTIVE_STATE_SHIFTER(ModOperatorName) \
	{ \
		for (auto& Modifier : AttributeToMod->##ModOperatorName##Mods) \
		{ \
			if (Modifier.ModSource == ModSource && Modifier.ModValue == ModValue) \
			{ \
				Modifier.bIsActive = bNewActiveState; \
				break; \
			} \
		} \
		break; \
	}

void UFireflyAbilitySystemComponent::ShiftModifierActiveState(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, bool bNewActiveState)
{
	if (!HasAuthority() || !IsValid(ModSource))
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
	{
		break;
	}
	case EFireflyAttributeModOperator::Plus:
	{
		FIREFLY_ATTRIBUTE_MODIFIER_ACTIVE_STATE_SHIFTER(Plus);
	}
	case EFireflyAttributeModOperator::Minus:
	{
		FIREFLY_ATTRIBUTE_MODIFIER_ACTIVE_STATE_SHIFTER(Minus);
	}
	case EFireflyAttributeModOperator::Multiply:
	{
		FIREFLY_ATTRIBUTE_MODIFIER_ACTIVE_STATE_SHIFTER(Multiply);
	}
	case EFireflyAttributeModOperator::Divide:
	{
		FIREFLY_ATTRIBUTE_MODIFIER_ACTIVE_STATE_SHIFTER(Divide);
	}
	case EFireflyAttributeModOperator::InnerOverride:
	{
		FIREFLY_ATTRIBUTE_MODIFIER_ACTIVE_STATE_SHIFTER(InnerOverride);
	}
	case EFireflyAttributeModOperator::OuterOverride:
	{
		FIREFLY_ATTRIBUTE_MODIFIER_ACTIVE_STATE_SHIFTER(OuterOverride);
	}
	}
}

bool UFireflyAbilitySystemComponent::CanApplyModifierInstant(EFireflyAttributeType AttributeType,
                                                             EFireflyAttributeModOperator ModOperator, float ModValue) const
{
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return false;
	}

	bool bResult = true;
	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
		{
			bResult = false;
		}
	case EFireflyAttributeModOperator::Plus:
	case EFireflyAttributeModOperator::Multiply:
		{
			break;
		}
	case EFireflyAttributeModOperator::Minus:
		{
			bResult = AttributeToMod->IsValueInAttributeRange(AttributeToMod->GetBaseValueToUse() - ModValue);
			break;
		}
	case EFireflyAttributeModOperator::Divide:
		{
			if (ModValue == 0.f)
			{
				break;
			}

			bResult = AttributeToMod->IsValueInAttributeRange(AttributeToMod->GetBaseValueToUse() / ModValue);
			break;
		}
	case EFireflyAttributeModOperator::InnerOverride:
	case EFireflyAttributeModOperator::OuterOverride:
		{
			bResult = AttributeToMod->IsValueInAttributeRange(AttributeToMod->GetBaseValueToUse());
			break;
		}
	}

	return bResult;
}

void UFireflyAbilitySystemComponent::ApplyModifierToAttributeInstant(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue)
{
	if (!HasAuthority() || !IsValid(ModSource))
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	PreModiferApplied(AttributeType, ModOperator, ModSource, ModValue, 1);

	AttributeToMod->UpdateBaseValue(ModOperator, ModValue);
	AttributeToMod->UpdateCurrentValue();

	PostModiferApplied(AttributeType, ModOperator, ModSource, ModValue, 1);
}

#define FIREFLY_ATTRIBUTE_MODIFIER_APPLY_OVERRIDE(ModOperatorName) \
	{ \
		bool bContainsModifier = false; \
		for (FFireflyAttributeModifier& Modifier : AttributeToMod->##ModOperatorName##Mods) \
		{ \
			if (Modifier.ModSource == ModSource) \
			{ \
				bContainsModifier = true; \
				Modifier.ModValue = ModValue; \
				Modifier.StackCount = StackToApply; \
				break; \
			} \
		} \
		if (!bContainsModifier) \
		{ \
			AttributeToMod->##ModOperatorName##Mods.AddUnique(FFireflyAttributeModifier(ModSource, ModValue, StackToApply)); \
		} \
		break; \
	}

void UFireflyAbilitySystemComponent::ApplyOrResetModifierToAttribute(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply)
{
	if (!HasAuthority() || !IsValid(ModSource))
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
		{
			break;
		}
	case EFireflyAttributeModOperator::Plus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY_OVERRIDE(Plus);
		}
	case EFireflyAttributeModOperator::Minus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY_OVERRIDE(Minus);
		}
	case EFireflyAttributeModOperator::Multiply:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY_OVERRIDE(Multiply);
		}
	case EFireflyAttributeModOperator::Divide:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY_OVERRIDE(Divide);
		}
	case EFireflyAttributeModOperator::InnerOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY_OVERRIDE(InnerOverride);
		}
	case EFireflyAttributeModOperator::OuterOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY_OVERRIDE(OuterOverride);
		}
	}

	PreModiferApplied(AttributeType, ModOperator, ModSource, ModValue, StackToApply);

	AttributeToMod->UpdateCurrentValue();

	PostModiferApplied(AttributeType, ModOperator, ModSource, ModValue, StackToApply);
}

TArray<UFireflyEffect*> UFireflyAbilitySystemComponent::GetActiveEffectsByID(FName EffectID) const
{
	if (EffectID == NAME_None)
	{
		return TArray<UFireflyEffect*>{};
	}

	TArray<UFireflyEffect*>  Effects;
	for (UFireflyEffect* Effect : ActiveEffects)
	{
		if (Effect->EffectID == EffectID)
		{
			Effects.Emplace(Effect);
		}
	}

	return Effects;
}

TArray<UFireflyEffect*> UFireflyAbilitySystemComponent::GetActiveEffectsByClass(
	TSubclassOf<UFireflyEffect> EffectType) const
{
	TArray<UFireflyEffect*> OutEffects = TArray<UFireflyEffect*>{};
	for (auto Effect : ActiveEffects)
	{
		if (Effect->GetClass() == EffectType)
		{
			OutEffects.Emplace(Effect);
		}
	}

	return OutEffects;
}

TArray<UFireflyEffect*> UFireflyAbilitySystemComponent::GetActiveEffectsByTag(
	FGameplayTagContainer EffectAssetTags) const
{
	if (!EffectAssetTags.IsValid())
	{
		return TArray<UFireflyEffect*>{};
	}

	TArray<UFireflyEffect*> OutEffects = TArray<UFireflyEffect*>{};
	for (auto Effect : ActiveEffects)
	{
		if (Effect->TagsForEffectAsset.HasAllExact(EffectAssetTags))
		{
			OutEffects.Emplace(Effect);
		}
	}

	return OutEffects;
}

FGameplayTagContainer UFireflyAbilitySystemComponent::GetBlockEffectTags() const
{
	FGameplayTagContainer OutTags;
	for (auto TagCount : BlockEffectTags)
	{
		OutTags.AddTag(TagCount.Key);
	}

	return OutTags;
}

void UFireflyAbilitySystemComponent::ApplyEffectToOwner(AActor* Instigator, UFireflyEffect* EffectInstance,
	int32 StackToApply)
{
	if (!IsValid(EffectInstance) || !HasAuthority() || StackToApply <= 0)
	{
		return;
	}

	/** 若效果会被阻挡，则应用无效 */
	if (EffectInstance->TagsForEffectAsset.HasAnyExact(GetBlockEffectTags())
		|| !EffectInstance->TagsRequireOwnerHasForApplication.HasAll(GetContainedTags())
		|| EffectInstance->TagsBlockApplicationOnOwnerHas.HasAnyExact(GetContainedTags()))
	{
		if (ActiveEffects.Contains(EffectInstance))
		{
			EffectInstance->MarkAsGarbage();
		}
		return;
	}

	if (!IsValid(Instigator))
	{
		Instigator = GetOwner();
	}

	const TArray<UFireflyEffect*> ActiveSpecEffects = GetActiveEffectsByClass(EffectInstance->GetClass());
	/** 管理器中目前如果不存在被应用的指定效果，则直接应用该效果实例 */
	if (ActiveSpecEffects.Num() == 0)
	{
		EffectInstance->ApplyEffect(Instigator, GetOwner(), StackToApply);

		return;
	}

	/** 如果指定效果的默认发起者应用策略为InstigatorsApplyTheirOwnOnly，不同的发起者仅生成各自的单个该效果实例 */
	if (EffectInstance->InstigatorApplicationPolicy == EFireflyEffectInstigatorApplicationPolicy::InstigatorsApplyTheirOwnOnly)
	{
		bool bContainsInstigator = false;
		for (auto Effect : ActiveSpecEffects)
		{
			if (Effect->Instigators.Contains(Instigator))
			{
				/** 已经存在的效果尝试应用堆叠或刷新操作 */
				Effect->ApplyEffect(Instigator, GetOwner(), StackToApply);

				bContainsInstigator = true;

				break;
			}
		}

		/** 如果指定效果在该管理器中目前生效的实例的发起者都不包含InInstigator，则应用新的效果实例 */
		if (!bContainsInstigator && !ActiveSpecEffects.Contains(EffectInstance))
		{
			EffectInstance->ApplyEffect(Instigator, GetOwner(), StackToApply);
		}		
	}
	/** 如果指定效果的默认发起者应用策略为InstigatorsShareOne，不同的发起者共享同一个该效果实例 */
	else if (EffectInstance->InstigatorApplicationPolicy == EFireflyEffectInstigatorApplicationPolicy::InstigatorsShareOne)
	{
		for (auto Effect : ActiveSpecEffects)
		{
			/** 若本次发起者为新的发起者，将该发起者加入效果实例中 */
			if (!Effect->Instigators.Contains(Instigator))
			{
				Effect->Instigators.AddUnique(Instigator);
			}

			/** 已经存在的效果尝试应用堆叠或刷新操作 */
			Effect->ApplyEffect(Instigator, GetOwner(), StackToApply);
		}
	}
	/** 如果指定效果的默认发起者应用策略为InstigatorsApplyTheirOwnMulti，不同的发起者可以生成多个不同的该效果实例 */
	else
	{
		EffectInstance->ApplyEffect(Instigator, GetOwner(), StackToApply);
	}

	if (ActiveEffects.Contains(EffectInstance))
	{
		EffectInstance->MarkAsGarbage();
	}
}

void UFireflyAbilitySystemComponent::ApplyEffectToTarget(AActor* Target, UFireflyEffect* EffectInstance,
	int32 StackToApply)
{
	if (!IsValid(Target) || !IsValid(EffectInstance) || StackToApply <= 0 || !HasAuthority())
	{
		return;
	}

	UFireflyAbilitySystemComponent* TargetEffectMgr = nullptr;
	if (!IsValid(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass())))
	{
		EffectInstance->MarkAsGarbage();
		return;
	}

	TargetEffectMgr = Cast<UFireflyAbilitySystemComponent>(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass()));
	TargetEffectMgr->ApplyEffectToOwner(GetOwner(), EffectInstance, StackToApply);
}

void UFireflyAbilitySystemComponent::ApplyEffectToOwnerByID(AActor* Instigator, FName EffectID, int32 StackToApply)
{
	if (EffectID == NAME_None)
	{
		return;
	}

	TSubclassOf<UFireflyEffect> EffectClass = UFireflyAbilitySystemLibrary::GetEffectClassFromCache(EffectID);
	if (!IsValid(EffectClass) || StackToApply <= 0 || !HasAuthority())
	{
		return;
	}

	if (!IsValid(Instigator))
	{
		Instigator = GetOwner();
	}

	UFireflyEffect* NewEffect = NewObject<UFireflyEffect>(this, EffectClass);
	ApplyEffectToOwner(Instigator, NewEffect, StackToApply);
}

void UFireflyAbilitySystemComponent::ApplyEffectToTargetByID(AActor* Target, FName EffectID, int32 StackToApply)
{
	if (!IsValid(Target) || StackToApply <= 0 || !HasAuthority() || EffectID == NAME_None)
	{
		return;
	}

	TSubclassOf<UFireflyEffect> EffectClass = UFireflyAbilitySystemLibrary::GetEffectClassFromCache(EffectID);
	if (!IsValid(EffectClass))
	{
		return;
	}

	UFireflyAbilitySystemComponent* TargetEffectMgr = nullptr;
	if (!IsValid(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass())))
	{
		return;
	}

	TargetEffectMgr = Cast<UFireflyAbilitySystemComponent>(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass()));
	TargetEffectMgr->ApplyEffectToOwnerByClass(GetOwner(), EffectClass, EffectID, StackToApply);
}

void UFireflyAbilitySystemComponent::ApplyEffectToOwnerByClass(AActor* Instigator, TSubclassOf<UFireflyEffect> EffectType,
	FName EffectID, int32 StackToApply)
{
	if (!IsValid(EffectType) || StackToApply <= 0 || !HasAuthority())
	{
		return;
	}

	if (!IsValid(Instigator))
	{
		Instigator = GetOwner();
	}

	UFireflyEffect* NewEffect = NewObject<UFireflyEffect>(this, EffectType);
	NewEffect->EffectID = EffectID;
	ApplyEffectToOwner(Instigator, NewEffect, StackToApply);
}

void UFireflyAbilitySystemComponent::ApplyEffectToTargetByClass(AActor* Target, TSubclassOf<UFireflyEffect> EffectType,
	FName EffectID, int32 StackToApply)
{
	if (!IsValid(Target) || !IsValid(EffectType) || StackToApply <= 0 || !HasAuthority())
	{
		return;
	}

	UFireflyAbilitySystemComponent* TargetEffectMgr = nullptr;
	if (!IsValid(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass())))
	{
		return;
	}

	TargetEffectMgr = Cast<UFireflyAbilitySystemComponent>(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass()));
	TargetEffectMgr->ApplyEffectToOwnerByClass(GetOwner(), EffectType, EffectID,StackToApply);
}

void UFireflyAbilitySystemComponent::ApplyEffectDynamicConstructorToOwner(AActor* Instigator,
	FFireflyEffectDynamicConstructor EffectSetup, int32 StackToApply)
{
	if (StackToApply <= 0 || !HasAuthority())
	{
		return;
	}

	UFireflyEffect* Effect = NewObject<UFireflyEffect>(this, IsValid(EffectSetup.EffectType) ? EffectSetup.EffectType : UFireflyEffect::StaticClass());
	Effect->SetupEffectByDynamicConstructor(EffectSetup);
	ApplyEffectToOwner(Instigator, Effect, StackToApply);
}

void UFireflyAbilitySystemComponent::ApplyEffectDynamicConstructorToTarget(AActor* Target,
	FFireflyEffectDynamicConstructor EffectSetup, int32 StackToApply)
{
	if (!IsValid(Target) || StackToApply <= 0 || !HasAuthority())
	{
		return;
	}

	UFireflyAbilitySystemComponent* TargetEffectMgr = nullptr;
	if (!IsValid(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass())))
	{
		return;
	}

	TargetEffectMgr = Cast<UFireflyAbilitySystemComponent>(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass()));
	TargetEffectMgr->ApplyEffectDynamicConstructorToOwner(GetOwner(), EffectSetup, StackToApply);
}

void UFireflyAbilitySystemComponent::RemoveActiveEffectsByID(FName EffectID, int32 StackToRemove)
{
	if (!HasAuthority() || EffectID == NAME_None)
	{
		return;
	}

	TArray<UFireflyEffect*> EffectsToRemove = TArray<UFireflyEffect*>{};
	for (auto Effect : ActiveEffects)
	{
		if (Effect->EffectID == EffectID)
		{
			EffectsToRemove.Add(Effect);
		}
	}

	if (StackToRemove == -1)
	{
		for (auto Effect : EffectsToRemove)
		{
			ActiveEffects.RemoveSingle(Effect);
			Effect->RemoveEffect();
		}

		return;
	}

	for (auto Effect : EffectsToRemove)
	{
		if (Effect->ReduceEffectStack(StackToRemove))
		{
			Effect->RemoveEffect();
		}
	}
}

void UFireflyAbilitySystemComponent::RemoveActiveEffectsByClass(TSubclassOf<UFireflyEffect> EffectType,
                                                               int32 StackToRemove)
{
	if (!IsValid(EffectType) || !HasAuthority())
	{
		return;
	}

	TArray<UFireflyEffect*> EffectsToRemove = TArray<UFireflyEffect*>{};
	for (auto Effect : ActiveEffects)
	{
		if (Effect->GetClass() == EffectType)
		{
			EffectsToRemove.Add(Effect);
		}
	}

	if (StackToRemove == -1)
	{
		for (auto Effect : EffectsToRemove)
		{
			ActiveEffects.RemoveSingle(Effect);
			Effect->RemoveEffect();
		}

		return;
	}

	for (auto Effect : EffectsToRemove)
	{
		if (Effect->ReduceEffectStack(StackToRemove))
		{
			Effect->RemoveEffect();
		}
	}
}

void UFireflyAbilitySystemComponent::RemoveActiveEffectsWithTags(FGameplayTagContainer RemoveTags)
{
	if (!RemoveTags.IsValid() || !HasAuthority())
	{
		return;
	}

	TArray<UFireflyEffect*> EffectsToRemove = TArray<UFireflyEffect*>{};
	for (auto Effect : ActiveEffects)
	{
		if (Effect->TagsForEffectAsset.HasAnyExact(RemoveTags))
		{
			EffectsToRemove.Add(Effect);
		}
	}

	for (auto Effect : EffectsToRemove)
	{
		Effect->RemoveEffect();
	}
}

void UFireflyAbilitySystemComponent::RemoveSingleActiveEffectByID(FName EffectID, int32 StackToRemove)
{
	if (!HasAuthority() || EffectID == NAME_None)
	{
		return;
	}

	UFireflyEffect* EffectToRemove = nullptr;
	for (auto Effect : ActiveEffects)
	{
		if (Effect->EffectID == EffectID)
		{
			EffectToRemove = Effect;
			break;
		}
	}

	if (StackToRemove == -1)
	{
		EffectToRemove->RemoveEffect();

		return;
	}

	if (EffectToRemove->ReduceEffectStack(StackToRemove))
	{
		EffectToRemove->RemoveEffect();
	}
}

void UFireflyAbilitySystemComponent::RemoveSingleActiveEffectByClass(TSubclassOf<UFireflyEffect> EffectType,
	int32 StackToRemove)
{
	if (!HasAuthority() || !IsValid(EffectType))
	{
		return;
	}

	UFireflyEffect* EffectToRemove = nullptr;
	for (auto Effect : ActiveEffects)
	{
		if (Effect->GetClass() == EffectType)
		{
			EffectToRemove = Effect;
			break;
		}
	}

	if (StackToRemove == -1)
	{
		EffectToRemove->RemoveEffect();

		return;
	}

	if (EffectToRemove->ReduceEffectStack(StackToRemove))
	{
		EffectToRemove->RemoveEffect();
	}
}

void UFireflyAbilitySystemComponent::HandleActiveEffectApplication(UFireflyEffect* InEffect, bool bIsApplied)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsApplied)
	{
		ActiveEffects.Emplace(InEffect);
		AppendEffectSpecificProperties(InEffect->SpecificProperties);
	}
	else
	{
		ActiveEffects.RemoveSingle(InEffect);
		RemoveEffectSpecificProperties(InEffect->SpecificProperties);
	}
}

void UFireflyAbilitySystemComponent::UpdateBlockAndRemoveEffectTags(FGameplayTagContainer BlockTags,
	FGameplayTagContainer RemoveTags, bool bIsApplied)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsApplied)
	{
		RemoveActiveEffectsWithTags(RemoveTags);

		TArray<FGameplayTag> Tags;
		BlockTags.GetGameplayTagArray(Tags);
		for (auto TagToAdd : Tags)
		{
			int32& Count = BlockEffectTags.FindOrAdd(TagToAdd);
			++Count;
		}
	}
	else
	{
		TArray<FGameplayTag> Tags;
		BlockTags.GetGameplayTagArray(Tags);
		for (auto TagToRemove : Tags)
		{
			if (!BlockEffectTags.Contains(TagToRemove))
			{
				continue;
			}

			int32* CountToMinus = BlockEffectTags.Find(TagToRemove);
			*CountToMinus = FMath::Clamp<int32>(*CountToMinus - 1, 0, *CountToMinus);

			if (*CountToMinus == 0)
			{
				BlockEffectTags.Remove(TagToRemove);
			}
		}
	}
}

bool UFireflyAbilitySystemComponent::GetSingleActiveEffectTimeDurationByID(FName EffectID, float& TimeRemaining,
	float& TotalDuration) const
{
	if (EffectID == NAME_None)
	{
		return false;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByID(EffectID);
	if (!Effects.IsValidIndex(0))
	{
		return false;
	}

	TimeRemaining = Effects[0]->GetTimeRemainingOfDuration();
	TotalDuration = Effects[0]->Duration;

	return true;
}

bool UFireflyAbilitySystemComponent::GetSingleActiveEffectTimeDurationByClass(TSubclassOf<UFireflyEffect> EffectType,
                                                                              float& TimeRemaining, float& TotalDuration) const
{
	if (!IsValid(EffectType))
	{
		return false;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByClass(EffectType);
	if (!Effects.IsValidIndex(0))
	{
		return false;
	}

	TimeRemaining = Effects[0]->GetTimeRemainingOfDuration();
	TotalDuration = Effects[0]->Duration;

	return true;
}

void UFireflyAbilitySystemComponent::SetActiveEffectsTimeRemaining(TSubclassOf<UFireflyEffect> EffectType,
	float NewTimeRemaining)
{
	if (!IsValid(EffectType) || !HasAuthority())
	{
		return;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByClass(EffectType);

	for (auto Effect : Effects)
	{
		Effect->SetTimeRemainingOfDuration(NewTimeRemaining);
	}	

	OnEffectTimeRemainingChanged.Broadcast(Effects[0]->EffectID, EffectType, NewTimeRemaining, Effects[0]->Duration);
}

bool UFireflyAbilitySystemComponent::GetSingleActiveEffectStackingCountByID(FName EffectID, int32& StackingCount) const
{
	if (EffectID == NAME_None)
	{
		return false;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByID(EffectID);
	if (!Effects.IsValidIndex(0))
	{
		return false;
	}

	StackingCount = Effects[0]->StackCount;

	return true;
}

bool UFireflyAbilitySystemComponent::GetSingleActiveEffectStackingCountByClass(TSubclassOf<UFireflyEffect> EffectType,
                                                                               int32& StackingCount) const
{
	if (!IsValid(EffectType))
	{
		return false;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByClass(EffectType);
	if (!Effects.IsValidIndex(0))
	{
		return false;
	}

	StackingCount = Effects[0]->StackCount;

	return true;
}

UFireflyEffect* UFireflyAbilitySystemComponent::MakeDynamicEffectByID(FName EffectID)
{
	if (EffectID == NAME_None)
	{
		return nullptr;
	}

	TSubclassOf<UFireflyEffect> EffectClass = UFireflyAbilitySystemLibrary::GetEffectClassFromCache(EffectID);
	if (!IsValid(EffectClass))
	{
		return nullptr;
	}

	UFireflyEffect* Effect = NewObject<UFireflyEffect>(this, EffectClass);
	Effect->EffectID = EffectID;

	return Effect;
}

UFireflyEffect* UFireflyAbilitySystemComponent::MakeDynamicEffectByClass(TSubclassOf<UFireflyEffect> EffectType, FName EffectID)
{
	if (!IsValid(EffectType))
	{
		return nullptr;
	}

	UFireflyEffect* Effect = NewObject<UFireflyEffect>(this, EffectType);
	Effect->EffectID = EffectID;

	return Effect;
}

UFireflyEffect* UFireflyAbilitySystemComponent::AssignDynamicEffectAssetTags(UFireflyEffect* EffectInstance,
	FGameplayTagContainer NewEffectAssetTags)
{
	EffectInstance->TagsForEffectAsset.AppendTags(NewEffectAssetTags);

	return EffectInstance;
}

UFireflyEffect* UFireflyAbilitySystemComponent::AssignDynamicEffectGrantTags(UFireflyEffect* EffectInstance,
	FGameplayTagContainer NewEffectGrantTags)
{
	EffectInstance->TagsApplyToOwnerOnApplied.AppendTags(NewEffectGrantTags);

	return EffectInstance;
}

UFireflyEffect* UFireflyAbilitySystemComponent::SetDynamicEffectDuration(UFireflyEffect* EffectInstance, float Duration)
{
	EffectInstance->Duration = Duration;

	return EffectInstance;
}

UFireflyEffect* UFireflyAbilitySystemComponent::SetDynamicEffectPeriodicInterval(UFireflyEffect* EffectInstance,
	float PeriodicInterval)
{
	EffectInstance->PeriodicInterval = PeriodicInterval;

	return EffectInstance;
}

UFireflyEffect* UFireflyAbilitySystemComponent::SetDynamicEffectModifierValue(UFireflyEffect* EffectInstance,
	EFireflyAttributeType AttributeType, EFireflyAttributeModOperator ModOperator, float ModValue)
{
	for (auto& EffectModifier : EffectInstance->Modifiers)
	{
		if (EffectModifier.AttributeType == AttributeType && EffectModifier.ModOperator == ModOperator)
		{
			EffectModifier.ModValue = ModValue;
			break;
		}
	}

	return EffectInstance;
}

UFireflyEffect* UFireflyAbilitySystemComponent::AssignDynamicEffectModifier(UFireflyEffect* EffectInstance,
	FFireflyEffectModifierData Modifier)
{
	bool bHasModifierType = false;
	for (auto& EffectModifier : EffectInstance->Modifiers)
	{
		if (EffectModifier.TypeEqual(Modifier))
		{
			EffectModifier = Modifier;
			bHasModifierType = true;
			break;
		}
	}

	if (!bHasModifierType)
	{
		EffectInstance->Modifiers.Emplace(Modifier);
	}

	return EffectInstance;
}

UFireflyEffect* UFireflyAbilitySystemComponent::AssignDynamicEffectSpecificProperty(UFireflyEffect* EffectInstance,
	FFireflySpecificProperty NewSpecificProperty)
{
	EffectInstance->SpecificProperties.Emplace(NewSpecificProperty);

	return EffectInstance;
}

void UFireflyAbilitySystemComponent::AppendEffectSpecificProperties(
	TArray<FFireflySpecificProperty> InSpecificProperties)
{
	for (auto SpecProperty : InSpecificProperties)
	{
		SpecificProperties.Emplace(SpecProperty);
	}
}

void UFireflyAbilitySystemComponent::RemoveEffectSpecificProperties(
	TArray<FFireflySpecificProperty> InSpecificProperties)
{
	for (auto SpecProperty : InSpecificProperties)
	{
		SpecificProperties.RemoveSingle(SpecProperty);
	}
}

FGameplayTagContainer UFireflyAbilitySystemComponent::GetContainedTags() const
{
	TArray<FGameplayTag> Tags;
	TagCountContainer.GetKeys(Tags);
	FGameplayTagContainer TagContainer = FGameplayTagContainer::CreateFromArray(Tags);

	return TagContainer;
}

void UFireflyAbilitySystemComponent::AddTagToManager(FGameplayTag TagToAdd, int32 CountToAdd)
{
	bool bContainedBefore = TagCountContainer.Contains(TagToAdd);
	int32& Count = TagCountContainer.FindOrAdd(TagToAdd);
	Count += CountToAdd;

	if (!bContainedBefore)
	{
		OnTagContainerUpdated.Broadcast(GetContainedTags());
	}
}

void UFireflyAbilitySystemComponent::RemoveTagFromManager(FGameplayTag TagToRemove, int32 CountToRemove)
{
	if (!TagCountContainer.Contains(TagToRemove))
	{
		return;
	}

	int32* CountToMinus = TagCountContainer.Find(TagToRemove);
	*CountToMinus = FMath::Clamp<int32>(*CountToMinus - CountToRemove, 0, *CountToMinus);

	if (*CountToMinus == 0)
	{
		TagCountContainer.Remove(TagToRemove);
		OnTagContainerUpdated.Broadcast(GetContainedTags());
	}
}

void UFireflyAbilitySystemComponent::AddTagsToManager(FGameplayTagContainer TagsToAdd, int32 CountToAdd)
{
	bool bContainedBefore = GetContainedTags().HasAll(TagsToAdd);

	TArray<FGameplayTag> TagsToUpdate;
	TagsToAdd.GetGameplayTagArray(TagsToUpdate);
	for (auto Tag : TagsToUpdate)
	{
		int32& Count = TagCountContainer.FindOrAdd(Tag);
		Count += CountToAdd;
	}

	if (!bContainedBefore)
	{
		OnTagContainerUpdated.Broadcast(GetContainedTags());
	}
}

void UFireflyAbilitySystemComponent::RemoveTagsFromManager(FGameplayTagContainer TagsToRemove, int32 CountToRemove)
{
	TArray<FGameplayTag> TagsToUpdate;
	TagsToRemove.GetGameplayTagArray(TagsToUpdate);

	TArray<FGameplayTag> TagsToClear;
	for (auto Tag : TagsToUpdate)
	{
		int32* CountToMinus = TagCountContainer.Find(Tag);
		*CountToMinus = FMath::Clamp<int32>(*CountToMinus - CountToRemove, 0, *CountToMinus);
		if (*CountToMinus == 0)
		{
			TagsToClear.AddUnique(Tag);
		}
	}

	if (TagsToClear.Num())
	{
		for (auto Tag : TagsToClear)
		{
			TagCountContainer.Remove(Tag);
		}
		OnTagContainerUpdated.Broadcast(GetContainedTags());
	}
}

void UFireflyAbilitySystemComponent::TryTriggerAbilityByMessage(UFireflyAbility* Ability,
	const FFireflyMessageEventData EventData)
{
	if (!IsValid(Ability))
	{
		return;
	}

	Server_TryTriggerAbilityByMessage(Ability, EventData);
}

void UFireflyAbilitySystemComponent::TriggerAbilityByMessage(UFireflyAbility* Ability,
	const FFireflyMessageEventData EventData)
{
	if (!IsValid(Ability))
	{
		return;
	}

	if (ActivatingAbilities.Contains(Ability))
	{
		return;
	}

	Ability->ActivateAbility();
	Ability->ActivateAbilityByMessage(EventData);
	ActivatingAbilities.Emplace(Ability);
}

void UFireflyAbilitySystemComponent::Server_TryTriggerAbilityByMessage_Implementation(UFireflyAbility* Ability,
	const FFireflyMessageEventData EventData)
{
	if (!IsValid(Ability))
	{
		return;
	}

	if (!Ability->CanActivateAbility())
	{
		return;
	}

	if (Ability->TagsForAbilityAsset.HasAnyExact(GetBlockAbilityTags()))
	{
		return;
	}

	TriggerAbilityByMessage(Ability, EventData);
	Client_TriggerAbilityByMessage(Ability, EventData);
}

void UFireflyAbilitySystemComponent::Client_TriggerAbilityByMessage_Implementation(UFireflyAbility* Ability,
	const FFireflyMessageEventData EventData)
{
	if (!IsValid(Ability))
	{
		return;
	}

	TriggerAbilityByMessage(Ability, EventData);
}

void UFireflyAbilitySystemComponent::HandleMessageEvent(FGameplayTag EventTag,
	FFireflyMessageEventData EventData)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	if (!EventData.EventTag.IsValid())
	{
		EventData.EventTag = EventTag;
	}

	OnReceiveMessageEvent.Broadcast(EventTag, EventData);

	for (auto Ability : GrantedAbilities)
	{
		if (!Ability->TagsTriggersActivation.HasTagExact(EventTag))
		{
			continue;
		}

		TryTriggerAbilityByMessage(Ability, EventData);
	}
}

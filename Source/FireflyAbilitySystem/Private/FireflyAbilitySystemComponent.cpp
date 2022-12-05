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

	// ...
}


// Called when the game starts
void UFireflyAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFireflyAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
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

void UFireflyAbilitySystemComponent::GrantAbilityByID(FName AbilityID)
{
	if (!HasAuthority())
	{
		return;
	}

	TSubclassOf<UFireflyAbility> AbilityToGrant = UFireflyAbilitySystemLibrary::GetAbilityClassFromDataTable(AbilityID);
	if (!IsValid(AbilityToGrant))
	{
		return;
	}

	if (IsValid(GetGrantedAbilityByClass(AbilityToGrant)))
	{
		return;
	}

	FName NewAbilityName = FName(AbilityToGrant->GetName() + FString("_") + GetOwner()->GetName());
	UFireflyAbility* NewAbility = NewObject<UFireflyAbility>(this, AbilityToGrant, NewAbilityName);
	NewAbility->AbilityID = AbilityID;
	NewAbility->OnAbilityGranted();
	GrantedAbilities.Emplace(NewAbility);
}

void UFireflyAbilitySystemComponent::GrantAbilityByClass(TSubclassOf<UFireflyAbility> AbilityToGrant, FName AbilityID)
{
	if (!IsValid(AbilityToGrant) || !HasAuthority())
	{
		return;
	}

	if (IsValid(GetGrantedAbilityByClass(AbilityToGrant)))
	{
		return;
	}

	FName NewAbilityName = FName(AbilityToGrant->GetName() + FString("_") + GetOwner()->GetName());
	UFireflyAbility* NewAbility = NewObject<UFireflyAbility>(this, AbilityToGrant, NewAbilityName);
	NewAbility->AbilityID = AbilityID;
	NewAbility->OnAbilityGranted();
	GrantedAbilities.Emplace(NewAbility);
}

void UFireflyAbilitySystemComponent::RemoveAbilityByID(FName AbilityID, bool bRemoveOnEnded)
{
	if (!HasAuthority())
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

void UFireflyAbilitySystemComponent::ActivateAbilityInternal(UFireflyAbility* Ability)
{
	Ability->ActivateAbility();
	ActivatingAbilities.Emplace(Ability);
}

void UFireflyAbilitySystemComponent::Server_TryActivateAbility_Implementation(UFireflyAbility* Ability)
{
	if (!Ability->CanActivateAbility() || !Ability->TagsForAbilityAsset.HasAnyExact(GetBlockAbilityTags()))
	{
		Ability->Client_CancelAbility();
		return;
	}

	ActivateAbilityInternal(Ability);
}

UFireflyAbility* UFireflyAbilitySystemComponent::TryActivateAbilityByID(FName AbilityID)
{
	UFireflyAbility* AbilityToActivate = GetGrantedAbilityByID(AbilityID);
	if (!IsValid(AbilityToActivate))
	{
		return nullptr;
	}

	return TryActivateAbilityByClass(AbilityToActivate->GetClass());
}

UFireflyAbility* UFireflyAbilitySystemComponent::TryActivateAbilityByClass(
	TSubclassOf<UFireflyAbility> AbilityToActivate)
{
	if (!IsValid(AbilityToActivate))
	{
		return nullptr;
	}

	UFireflyAbility* Ability = GetGrantedAbilityByClass(AbilityToActivate);
	if (!IsValid(Ability))
	{
		return nullptr;
	}

	if (!Ability->CanActivateAbility())
	{
		return nullptr;
	}

	if (Ability->TagsForAbilityAsset.HasAnyExact(GetBlockAbilityTags()))
	{
		return nullptr;
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		ActivateAbilityInternal(Ability);
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{		
		Server_TryActivateAbility(Ability);
	}

	return Ability;
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
	UInputAction* InputToBind, bool bForceBind)
{
	if (!IsValid(InputToBind) || !IsValid(AbilityToBind))
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

	if (!IsValid(GetGrantedAbilityByClass(AbilityToBind)) && !bForceBind)
	{
		return;
	}

	FFireflyAbilitiesBoundToInput& AbilitiesBoundToInput = AbilitiesInputBound.FindOrAdd(InputToBind);
	if (AbilitiesBoundToInput.Abilities.Num() == 0)
	{
		AbilitiesBoundToInput.HandleStarted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Started, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionStarted, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleOngoing = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Ongoing, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionOngoing, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleCanceled = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Canceled, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionCanceled, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleTriggered = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Triggered, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionTriggered, InputToBind).GetHandle();
		AbilitiesBoundToInput.HandleCompleted = EnhancedInput->BindAction(InputToBind, ETriggerEvent::Completed, this,
			&UFireflyAbilitySystemComponent::OnAbilityInputActionCompleted, InputToBind).GetHandle();
	}

	AbilitiesBoundToInput.Abilities.AddUnique(AbilityToBind);
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
	if (AbilitiesBoundToInput->Abilities.Num() == 0)
	{
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleStarted);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleOngoing);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleCanceled);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleTriggered);
		EnhancedInput->RemoveBindingByHandle(AbilitiesBoundToInput->HandleCompleted);

		AbilitiesInputBound.Remove(InputToUnbind);
	}
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

float UFireflyAbilitySystemComponent::GetAttributeValue(EFireflyAttributeType AttributeType) const
{
	float OutValue = 0.f;
	for (auto Attribute : AttributeContainer)
	{
		if (Attribute->AttributeType == AttributeType)
		{
			OutValue = Attribute->GetCurrentValue();
			break;
		}
	}

	return OutValue;
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

	FString NewAttributeName = UFireflyAbilitySystemLibrary::GetAttributeTypeName(AttributeConstructor.AttributeType) + FString("_") + (TEXT("%s"), GetOwner()->GetName());

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, *NewAttributeName);
	NewAttribute->AttributeType = AttributeConstructor.AttributeType;
	NewAttribute->Initialize(AttributeConstructor.AttributeInitValue);
	NewAttribute->bAttributeHasRange = AttributeConstructor.bAttributeHasRange;
	NewAttribute->RangeMinValue = AttributeConstructor.RangeMinValue;
	NewAttribute->RangeMaxValue = AttributeConstructor.RangeMaxValue;
	NewAttribute->RangeMaxValueType = AttributeConstructor.RangeMaxValueType;

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::ConstructAttributeByClass(TSubclassOf<UFireflyAttribute> AttributeClass,
	float InitValue)
{
	if (!IsValid(AttributeClass) || !HasAuthority())
	{
		return;
	}

	EFireflyAttributeType AttributeType = Cast<UFireflyAttribute>(AttributeClass->GetDefaultObject())->AttributeType;
	FString NewAttributeName = UFireflyAbilitySystemLibrary::GetAttributeTypeName(AttributeType) + FString("_") + (TEXT("%s"), GetOwner()->GetName());

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, AttributeClass, *NewAttributeName);
	NewAttribute->AttributeType = AttributeType;
	NewAttribute->Initialize(InitValue);

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::ConstructAttributeByType(EFireflyAttributeType AttributeType, float InitValue)
{
	if (!HasAuthority())
	{
		return;
	}

	FString NewAttributeName = UFireflyAbilitySystemLibrary::GetAttributeTypeName(AttributeType) + FString("_") + (TEXT("%s"), GetOwner()->GetName());

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, *NewAttributeName);
	NewAttribute->AttributeType = AttributeType;
	NewAttribute->Initialize(InitValue);

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::InitializeAttribute(EFireflyAttributeType AttributeType, float NewInitValue)
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

	AttributeToInit->BaseValue = NewInitValue;
	AttributeToInit->CurrentValue = NewInitValue;
}

void UFireflyAbilitySystemComponent::ApplyModifierToAttribute(EFireflyAttributeType AttributeType,
                                                              EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue, int32 StackToApply)
{
	if (!HasAuthority())
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
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
			break;
		}
	case EFireflyAttributeModOperator::Minus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(Minus);
			break;
		}
	case EFireflyAttributeModOperator::Multiply:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(Multiply);
			break;
		}
	case EFireflyAttributeModOperator::Divide:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(Divide);
			break;
		}
	case EFireflyAttributeModOperator::InnerOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(InnerOverride);
			break;
		}
	case EFireflyAttributeModOperator::OuterOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_APPLY(OuterOverride);
			break;
		}
	}

	AttributeToMod->UpdateCurrentValue();
}

void UFireflyAbilitySystemComponent::RemoveModifierFromAttribute(EFireflyAttributeType AttributeType,
	EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue)
{
	if (!HasAuthority())
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	FFireflyAttributeModifier ModifierToRemove = FFireflyAttributeModifier(ModSource, ModValue);

#define FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(ModOperatorName) \
	if (AttributeToMod->##ModOperatorName##Mods.Contains(ModifierToRemove)) \
	{ \
		AttributeToMod->##ModOperatorName##Mods.RemoveSingle(ModifierToRemove); \
	}

	switch (ModOperator)
	{
	case EFireflyAttributeModOperator::None:
		{
			break;
		}
	case EFireflyAttributeModOperator::Plus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Plus);
			break;
		}
	case EFireflyAttributeModOperator::Minus:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Minus);
			break;
		}
	case EFireflyAttributeModOperator::Multiply:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Multiply);
			break;
		}
	case EFireflyAttributeModOperator::Divide:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(Divide);
			break;
		}
	case EFireflyAttributeModOperator::InnerOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(InnerOverride);
			break;
	}
	case EFireflyAttributeModOperator::OuterOverride:
		{
			FIREFLY_ATTRIBUTE_MODIFIER_REMOVE(OuterOverride);
			break;
		}
	}

	AttributeToMod->UpdateCurrentValue();
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
			bResult = AttributeToMod->IsValueInAttributeRange(AttributeToMod->BaseValue - ModValue);
			break;
		}
	case EFireflyAttributeModOperator::Divide:
		{
			if (ModValue == 0.f)
			{
				break;
			}

			bResult = AttributeToMod->IsValueInAttributeRange(AttributeToMod->BaseValue / ModValue);
			break;
		}
	case EFireflyAttributeModOperator::InnerOverride:
	case EFireflyAttributeModOperator::OuterOverride:
		{
			bResult = AttributeToMod->IsValueInAttributeRange(AttributeToMod->BaseValue);
			break;
		}
	}

	return bResult;
}

void UFireflyAbilitySystemComponent::ApplyModifierToAttributeInstant(EFireflyAttributeType AttributeType,
                                                                     EFireflyAttributeModOperator ModOperator, UObject* ModSource, float ModValue)
{
	if (!HasAuthority())
	{
		return;
	}

	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->UpdateBaseValue(ModOperator, ModValue);
}

TArray<UFireflyEffect*> UFireflyAbilitySystemComponent::GetActiveEffectsByID(FName EffectID) const
{
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

	if (!IsValid(Instigator))
	{
		Instigator = GetOwner();
	}

	/** 若效果会被阻挡，则应用无效 */
	if (EffectInstance->TagsForEffectAsset.HasAnyExact(GetBlockEffectTags())
		|| !EffectInstance->TagsRequireOwnerHasForApplication.HasAll(GetContainedTags())
		|| EffectInstance->TagsBlockApplicationOnOwnerHas.HasAnyExact(GetContainedTags()))
	{
		if (!ActiveEffects.Contains(EffectInstance))
		{
			EffectInstance->MarkAsGarbage();
		}
		return;
	}

	const TArray<UFireflyEffect*> ActiveSpecEffects = GetActiveEffectsByClass(EffectInstance->GetClass());

	/** 管理器中目前如果不存在被应用的指定效果，则直接应用该效果实例 */
	if (ActiveSpecEffects.Num() == 0)
	{
		EffectInstance->ApplyEffect(Instigator, GetOwner(), StackToApply);

		return;
	}

	/** 如果指定效果的默认发起者应用策略为每个发起者应用各自的实例 */
	if (EffectInstance->InstigatorApplicationPolicy == EFireflyEffectInstigatorApplicationPolicy::InstigatorsApplyTheirOwn)
	{
		bool bContainsInstigator = false;
		for (auto Effect : ActiveSpecEffects)
		{
			if (Effect->Instigators.Contains(Instigator))
			{
				bContainsInstigator = true;
				break;
			}
		}

		// 如果指定效果在该管理器中目前不存在已经存在的，和InInstigator相同的发起者，则应用该效果实例
		if (!bContainsInstigator)
		{
			EffectInstance->ApplyEffect(Instigator, GetOwner(), StackToApply);

			return;
		}
	}

	/** 不考虑特殊情况，将所有当前存在的执行效果重新应用，具体的判定逻辑在效果内部执行 */
	for (auto Effect : ActiveSpecEffects)
	{
		Effect->ApplyEffect(Instigator, GetOwner(), StackToApply);
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
		return;
	}

	TargetEffectMgr = Cast<UFireflyAbilitySystemComponent>(Target->GetComponentByClass(UFireflyAbilitySystemComponent::StaticClass()));
	TargetEffectMgr->ApplyEffectToOwner(GetOwner(), EffectInstance, StackToApply);
}

void UFireflyAbilitySystemComponent::ApplyEffectToOwnerByID(AActor* Instigator, FName EffectID, int32 StackToApply)
{
	TSubclassOf<UFireflyEffect> EffectClass = UFireflyAbilitySystemLibrary::GetEffectClassFromDataTable(EffectID);
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
	if (!IsValid(Target) || StackToApply <= 0 || !HasAuthority())
	{
		return;
	}

	TSubclassOf<UFireflyEffect> EffectClass = UFireflyAbilitySystemLibrary::GetEffectClassFromDataTable(EffectID);
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

void UFireflyAbilitySystemComponent::RemoveActiveEffectByID(FName EffectID, int32 StackToRemove)
{
	if (!HasAuthority())
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
	}

	for (auto Effect : EffectsToRemove)
	{
		if (Effect->ReduceEffectStack(StackToRemove))
		{
			ActiveEffects.RemoveSingle(Effect);
			Effect->RemoveEffect();
		}
	}
}

void UFireflyAbilitySystemComponent::RemoveActiveEffectByClass(TSubclassOf<UFireflyEffect> EffectType,
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
	}

	for (auto Effect : EffectsToRemove)
	{
		if (Effect->ReduceEffectStack(StackToRemove))
		{
			ActiveEffects.RemoveSingle(Effect);
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
		ActiveEffects.RemoveSingle(Effect);
		Effect->RemoveEffect();
	}
}

void UFireflyAbilitySystemComponent::AddOrRemoveActiveEffect(UFireflyEffect* InEffect, bool bIsAdd)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsAdd)
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

void UFireflyAbilitySystemComponent::SetSingleActiveEffectTimeRemaining(TSubclassOf<UFireflyEffect> EffectType,
	float NewTimeRemaining)
{
	if (!IsValid(EffectType) || !HasAuthority())
	{
		return;
	}

	const TArray<UFireflyEffect*> Effects = GetActiveEffectsByClass(EffectType);
	if (!Effects.IsValidIndex(0))
	{
		return;
	}

	Effects[0]->SetTimeRemainingOfDuration(NewTimeRemaining);

	OnEffectTimeRemainingChanged.Broadcast(Effects[0]->EffectID, EffectType, NewTimeRemaining, Effects[0]->Duration);
}

bool UFireflyAbilitySystemComponent::GetSingleActiveEffectStackingCountByID(FName EffectID, int32& StackingCount) const
{
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
	TSubclassOf<UFireflyEffect> EffectClass = UFireflyAbilitySystemLibrary::GetEffectClassFromDataTable(EffectID);
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

UFireflyEffect* UFireflyAbilitySystemComponent::AssignDynamicEffectModifiers(UFireflyEffect* EffectInstance,
	FFireflyEffectModifierData Modifier)
{
	bool bHasModifierType = false;
	for (auto EffectModifier : EffectInstance->Modifiers)
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

UFireflyEffect* UFireflyAbilitySystemComponent::AssignDynamicEffectSpecificProperties(UFireflyEffect* EffectInstance,
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

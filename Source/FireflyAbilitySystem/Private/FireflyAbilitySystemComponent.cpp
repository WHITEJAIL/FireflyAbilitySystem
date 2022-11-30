// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyAbilitySystemComponent.h"

#include "EnhancedInputComponent.h"
#include "FireflyAbility.h"
#include "FireflyAttribute.h"
#include "FireflyEffect.h"
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

bool UFireflyAbilitySystemComponent::IsOwnerLocallyControlled() const
{
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

void UFireflyAbilitySystemComponent::GrantAbility(TSubclassOf<UFireflyAbility> AbilityToGrant)
{
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
	NewAbility->OnAbilityGranted();
	GrantedAbilities.Emplace(NewAbility);
}

void UFireflyAbilitySystemComponent::RemoveAbility(TSubclassOf<UFireflyAbility> AbilityToRemove)
{
	if (!IsValid(AbilityToRemove))
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
		Ability->CancelAbility();
	}
	GrantedAbilities.RemoveSingle(Ability);
	Ability->MarkAsGarbage();
}

void UFireflyAbilitySystemComponent::RemoveAbilityOnEnded(TSubclassOf<UFireflyAbility> AbilityToRemove)
{
	if (!IsValid(AbilityToRemove))
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
		Ability->bRemoveOnEndedExecution = true;
	}
	else
	{
		GrantedAbilities.RemoveSingle(Ability);
		Ability->MarkAsGarbage();
	}
}

void UFireflyAbilitySystemComponent::Server_TryActivateAbility_Implementation(UFireflyAbility* AbilityToActivate,
																			  bool bNeedValidation)
{
	if (bNeedValidation)
	{
		if (!AbilityToActivate->CanActivateAbility())
		{
			AbilityToActivate->Client_CancelAbility();
			return;
		}

		TArray<FGameplayTag> BlockTagArray;
		BlockAbilityTags.GetKeys(BlockTagArray);
		FGameplayTagContainer BlockTags = FGameplayTagContainer::CreateFromArray(BlockTagArray);
		if (AbilityToActivate->TagsForAbilityAsset.HasAnyExact(BlockTags))
		{
			AbilityToActivate->Client_CancelAbility();
			return;
		}
	}

	AbilityToActivate->ActivateAbility();
	ActivatingAbilities.Emplace(AbilityToActivate);
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

	TArray<FGameplayTag> BlockTagArray;
	BlockAbilityTags.GetKeys(BlockTagArray);
	FGameplayTagContainer BlockTags = FGameplayTagContainer::CreateFromArray(BlockTagArray);
	if (Ability->TagsForAbilityAsset.HasAnyExact(BlockTags))
	{
		return nullptr;
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_TryActivateAbility(Ability, false);
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Ability->ActivateAbility();
		ActivatingAbilities.Emplace(Ability);
		Server_TryActivateAbility(Ability, true);
	}

	return Ability;
}

void UFireflyAbilitySystemComponent::CancelAbilitiesWithTags(FGameplayTagContainer CancelTags)
{
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

void UFireflyAbilitySystemComponent::UpdateBlockAndCancelAbilityTags(FGameplayTagContainer BlockTags,
	FGameplayTagContainer CancelTags, bool bIsActivated)
{
	if (bIsActivated)
	{
		CancelAbilitiesWithTags(CancelTags);

		TArray<FGameplayTag> Tags;
		CancelTags.GetGameplayTagArray(Tags);
		for (auto TagToAdd : Tags)
		{
			int32& Count = BlockAbilityTags.FindOrAdd(TagToAdd);
			++Count;
		}
	}
	else
	{
		TArray<FGameplayTag> Tags;
		CancelTags.GetGameplayTagArray(Tags);
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
	if (!IsValid(InputToBind) || !IsValid(AbilityToBind))
	{
		return;
	}

	if (!IsOwnerLocallyControlled())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInput = GetEnhancedInputComponentFromOwner();
	if (!IsValid(EnhancedInput))
	{
		return;
	}

	if (!IsValid(GetGrantedAbilityByClass(AbilityToBind)))
	{
		return;
	}

	UFireflyAbility* Ability = Cast<UFireflyAbility>(GetGrantedAbilityByClass(AbilityToBind));
	if (!IsValid(Ability))
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

	if (!IsOwnerLocallyControlled())
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

FString UFireflyAbilitySystemComponent::GetAttributeTypeName(EFireflyAttributeType AttributeType) const
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFireflyAttributeType"), true);
	if (!EnumPtr) return FString("Invalid");

	return EnumPtr != nullptr ? EnumPtr->GetDisplayNameTextByValue(AttributeType).ToString() : FString("Invalid");
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

void UFireflyAbilitySystemComponent::ConstructAttribute(FFireflyAttributeConstructor AttributeConstructor)
{
	FString NewAttributeName = *GetAttributeTypeName(AttributeConstructor.AttributeType) + FString("_") + (TEXT("%s"), GetOwner()->GetName());

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, *NewAttributeName);
	NewAttribute->AttributeType = AttributeConstructor.AttributeType;
	NewAttribute->Initialize(AttributeConstructor.AttributeInitValue);
	NewAttribute->bAttributeHasRange = AttributeConstructor.bAttributeHasRange;
	NewAttribute->RangeMinValue = AttributeConstructor.RangeMinValue;
	NewAttribute->RangeMaxValue = AttributeConstructor.RangeMaxValue;
	NewAttribute->RangeMaxValueType = AttributeConstructor.RangeMaxValueType;

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::ConstructAttributeByType(EFireflyAttributeType AttributeType, float InitValue)
{
	FString NewAttributeName = *GetAttributeTypeName(AttributeType) + FString("_") + (TEXT("%s"), GetOwner()->GetName());

	UFireflyAttribute* NewAttribute = NewObject<UFireflyAttribute>(this, *NewAttributeName);
	NewAttribute->AttributeType = AttributeType;
	NewAttribute->Initialize(InitValue);

	AttributeContainer.Emplace(NewAttribute);
}

void UFireflyAbilitySystemComponent::InitializeAttribute(EFireflyAttributeType AttributeType, float NewInitValue)
{
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
	UFireflyAttribute* AttributeToMod = GetAttributeByType(AttributeType);
	if (!IsValid(AttributeToMod))
	{
		return;
	}

	AttributeToMod->UpdateBaseValue(ModOperator, ModValue);
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

void UFireflyAbilitySystemComponent::ApplyEffectToSelf(AActor* Instigator, TSubclassOf<UFireflyEffect> EffectType,
	int32 StackToApply)
{
	if (!IsValid(EffectType) || StackToApply <= 0)
	{
		return;
	}

	if (!IsValid(Instigator))
	{
		Instigator = GetOwner();
	}

	const TArray<UFireflyEffect*> ActiveSpecEffects = GetActiveEffectsByClass(EffectType);

	/** 管理器中目前如果不存在被应用的指定效果，则创建一个新效果，应用该效果 */
	if (ActiveSpecEffects.Num() == 0)
	{
		UFireflyEffect* NewEffect = NewObject<UFireflyEffect>(this, EffectType);
		NewEffect->ApplyEffect(Instigator, GetOwner(), StackToApply);

		return;
	}

	/** 如果指定效果的默认发起者应用策略为每个发起者应用各自的实例 */
	UFireflyEffect* EffectCDO = Cast<UFireflyEffect>(EffectType->GetDefaultObject());
	if (EffectCDO->InstigatorApplicationPolicy == EFireflyEffectInstigatorApplicationPolicy::InstigatorsApplyTheirOwn)
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

		// 如果指定效果在该管理器中目前不存在已经存在的，和InInstigator相同的发起者，则创建一个新效果，应用该效果
		if (!bContainsInstigator)
		{
			UFireflyEffect* NewEffect = NewObject<UFireflyEffect>(this, EffectType);
			NewEffect->ApplyEffect(Instigator, GetOwner(), StackToApply);

			return;
		}
	}

	/** 不考虑特殊情况，将所有当前存在的执行效果重新应用，具体的判定逻辑在效果内部执行 */
	for (auto Effect : ActiveSpecEffects)
	{
		Effect->ApplyEffect(Instigator, GetOwner(), StackToApply);
	}
}

void UFireflyAbilitySystemComponent::ApplyEffectToTarget(AActor* Target, TSubclassOf<UFireflyEffect> EffectType,
	int32 StackToApply)
{
	if (!IsValid(Target) || !IsValid(EffectType) || StackToApply <= 0)
	{
		return;
	}

	UFireflyAbilitySystemComponent* TargetEffectMgr = nullptr;
	if (!IsValid(Target->GetComponentByClass(GetClass())))
	{
		return;
	}

	TargetEffectMgr = Cast<UFireflyAbilitySystemComponent>(Target->GetComponentByClass(GetClass()));
	TargetEffectMgr->ApplyEffectToSelf(GetOwner(), EffectType, StackToApply);
}

void UFireflyAbilitySystemComponent::RemoveActiveEffectFromSelf(TSubclassOf<UFireflyEffect> EffectType,
	int32 StackToRemove)
{
	if (!IsValid(EffectType))
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

	for (auto Effect : EffectsToRemove)
	{
		if (Effect->ReduceEffectStack(StackToRemove))
		{
			EffectsToRemove.RemoveSingle(Effect);
			Effect->RemoveEffect();
		}
	}
}

void UFireflyAbilitySystemComponent::AddOrRemoveActiveEffect(UFireflyEffect* InEffect, bool bIsAdd)
{
	if (bIsAdd)
	{
		ActiveEffects.Emplace(InEffect);
	}
	else
	{
		ActiveEffects.RemoveSingle(InEffect);
	}
}

bool UFireflyAbilitySystemComponent::GetSingleActiveEffectTimeDuration(TSubclassOf<UFireflyEffect> EffectType,
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

bool UFireflyAbilitySystemComponent::GetSingleActiveEffectStackingCount(TSubclassOf<UFireflyEffect> EffectType,
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

FGameplayTagContainer UFireflyAbilitySystemComponent::GetContainedTags() const
{
	TArray<FGameplayTag> Tags;
	TagCountContainer.GetKeys(Tags);
	FGameplayTagContainer TagContainer = FGameplayTagContainer::CreateFromArray(Tags);

	return TagContainer;
}

void UFireflyAbilitySystemComponent::AddTagToManager(FGameplayTag TagToAdd, int32 CountToAdd)
{
	int32& Count = TagCountContainer.FindOrAdd(TagToAdd);
	Count += CountToAdd;
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
	}
}

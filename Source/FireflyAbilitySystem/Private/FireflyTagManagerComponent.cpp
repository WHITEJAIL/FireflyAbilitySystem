// Fill out your copyright notice in the Description page of Project Settings.


#include "FireflyTagManagerComponent.h"

// Sets default values for this component's properties
UFireflyTagManagerComponent::UFireflyTagManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFireflyTagManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFireflyTagManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FGameplayTagContainer UFireflyTagManagerComponent::GetContainedTags() const
{
	TArray<FGameplayTag> Tags;
	TagCountContainer.GetKeys(Tags);
	FGameplayTagContainer TagContainer = FGameplayTagContainer::CreateFromArray(Tags);

	return TagContainer;
}

void UFireflyTagManagerComponent::AddTagToManager(FGameplayTag TagToAdd, int32 CountToAdd)
{
	int32& Count = TagCountContainer.FindOrAdd(TagToAdd);
	Count += CountToAdd;
}

void UFireflyTagManagerComponent::RemoveTagFromManager(FGameplayTag TagToRemove, int32 CountToRemove)
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

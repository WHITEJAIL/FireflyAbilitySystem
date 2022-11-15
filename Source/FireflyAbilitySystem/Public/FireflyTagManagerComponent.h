// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FireflyTagManagerComponent.generated.h"

/** GameplayTag管理器组件 */
UCLASS( ClassGroup=(FireflyAbilitySystem), meta=(BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyTagManagerComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

#pragma region Override

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

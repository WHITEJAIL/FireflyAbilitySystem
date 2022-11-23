// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FireflyEffectManagerComponent.generated.h"

class UFireflyEffect;
/** 效果管理器组件 */
UCLASS( ClassGroup=(FireflyAbilitySystem), meta=(BlueprintSpawnableComponent) )
class FIREFLYABILITYSYSTEM_API UFireflyEffectManagerComponent : public UActorComponent
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


#pragma region Application

protected:
	/** 在该管理器中获取特定类型的被应用的激活中的效果 */
	UFUNCTION()
	UFireflyEffect* GetActiveEffectByClass(TSubclassOf<UFireflyEffect> EffectType) const;

public:
	/** 该效果是否在该管理器中处于被应用的激活状态 */
	UFUNCTION(BlueprintPure, Category = "FireflyAbilitySystem|Effect")
	bool IsEffectActive(TSubclassOf<UFireflyEffect> EffectType) const;

	/** 为自身应用效果 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToSelf(AActor* Instigator, TSubclassOf<UFireflyEffect> EffectType, int32 StackToApply = 1);

	/** 为目标应用效果 */
	UFUNCTION(BlueprintCallable, Category = "FireflyAbilitySystem|Effect")
	void ApplyEffectToTarget(AActor* Target, TSubclassOf<UFireflyEffect> EffectType, int32 StackToApply = 1);

protected:
	/** 所有激活中的执行策略不是Instant的效果 */
	UPROPERTY()
	TArray<UFireflyEffect*> ActiveEffects;

#pragma endregion
		
};

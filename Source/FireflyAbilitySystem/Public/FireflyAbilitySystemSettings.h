// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FireflyAbilitySystemSettings.generated.h"

/**
 * 代表技能系统使用的属性名称的结构体
 */
USTRUCT()
struct FFireflyAttributeTypeName
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TEnumAsByte<enum EFireflyAttributeType> Type;

	UPROPERTY()
	FName Name;

	FFireflyAttributeTypeName()	: Type(SurfaceType_Max) {}

	FFireflyAttributeTypeName(EFireflyAttributeType InType, const FName& InName) : Type(InType), Name(InName) {}
};

/**
 * 个人技能系统的设置
 */
UCLASS(Config = Game, DefaultConfig, Meta = (DisplayName = "Firefly Ability System"))
class FIREFLYABILITYSYSTEM_API UFireflyAbilitySystemSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()

public:
	// PhysicalMaterial Surface Types
	UPROPERTY(Config, EditAnywhere, Category = AttributeTypes)
	TArray<FFireflyAttributeTypeName> AttributeTypes;

public:
	static UFireflyAbilitySystemSettings* Get();

	virtual void PostInitProperties() override;

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return FName("Project"); }

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return FName("Game"); }

	/** The unique name for your section of settings, uses the class's FName. */
	virtual FName GetSectionName() const override { return FName("Firefly Ability System Settings"); }

#if WITH_EDITOR
	/** Load Attribute Type data from INI file **/
	/** this changes displayname meta data. That means we won't need it outside of editor*/
	void LoadSurfaceType();
#endif	
};

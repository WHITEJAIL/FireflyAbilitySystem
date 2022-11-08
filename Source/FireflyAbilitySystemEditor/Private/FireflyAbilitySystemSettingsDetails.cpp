// Copyright Epic Games, Inc. All Rights Reserved.

#include "FireflyAbilitySystemSettingsDetails.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SListView.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "FireflyAbilitySystemTypes.h"
#include "Widgets/SToolTip.h"
#include "IDocumentation.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "FireflyAbilitySystemDetails"

void SFireflyAttributeTypeEditBox::Construct(const FArguments& InArgs)
{
	AttributeType = InArgs._AttributeType;
	AttributeTypeEnum = InArgs._AttributeTypeEnum;
	OnCommitChange = InArgs._OnCommitChange;
	check(AttributeType.IsValid() && AttributeTypeEnum);

	ChildSlot
		[
			SAssignNew(NameEditBox, SEditableTextBox)
			.Text(this, &SFireflyAttributeTypeEditBox::GetName)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.OnTextCommitted(this, &SFireflyAttributeTypeEditBox::NewNameEntered)
		.OnTextChanged(this, &SFireflyAttributeTypeEditBox::OnTextChanged)
		.IsReadOnly(AttributeType->Type == AttributeType_Default)
		.SelectAllTextWhenFocused(true)
		];
}



void SFireflyAttributeTypeEditBox::OnTextChanged(const FText& NewText)
{
	FString NewName = NewText.ToString();

	if (NewName.Find(TEXT(" ")) != INDEX_NONE)
	{
		// no white space
		NameEditBox->SetError(TEXT("No white space is allowed"));
	}
	else
	{
		NameEditBox->SetError(TEXT(""));
	}
}

void SFireflyAttributeTypeEditBox::NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo)
{
	// Don't digest the number if we just clicked away from the pop-up
	if ((CommitInfo == ETextCommit::OnEnter) || (CommitInfo == ETextCommit::OnUserMovedFocus))
	{
		FString NewName = NewText.ToString();
		if (NewName.Find(TEXT(" ")) == INDEX_NONE)
		{
			FName NewAttributeTypeName(*NewName);
			if (AttributeType->Name != NAME_None && NewAttributeTypeName == NAME_None)
			{
				if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SAttributeTypeListItem_DeleteConfirm", "Would you like to delete the name? If this type is used, it will invalidate the usage.")) == EAppReturnType::No)
				{
					return;
				}
			}
			if (NewAttributeTypeName != AttributeType->Name)
			{
				AttributeType->Name = NewAttributeTypeName;
				OnCommitChange.ExecuteIfBound();
			}
		}
		else
		{
			// clear error
			NameEditBox->SetError(TEXT(""));
		}
	}
}

FText SFireflyAttributeTypeEditBox::GetName() const
{
	return FText::FromName(AttributeType->Name);
}

class FAttributeTypeList : public IDetailCustomNodeBuilder, public TSharedFromThis<FAttributeTypeList>
{
public:
	FAttributeTypeList(UFireflyAbilitySystemSettings* InFireflyAbilitySystemSettings, UEnum* InAttributeTypeEnum, TSharedPtr<IPropertyHandle>& InAttributeTypesProperty)
		: FireflyAbilitySystemSettings(InFireflyAbilitySystemSettings)
		, AttributeTypeEnum(InAttributeTypeEnum)
		, AttributeTypesProperty(InAttributeTypesProperty)
	{
		AttributeTypesProperty->MarkHiddenByCustomization();
	}

	void RefreshAttributeTypeList()
	{
		// make sure no duplicate exists
		// if exists, use the last one
		for (auto Iter = FireflyAbilitySystemSettings->AttributeTypes.CreateIterator(); Iter; ++Iter)
		{
			for (auto InnerIter = Iter + 1; InnerIter; ++InnerIter)
			{
				// see if same type
				if (Iter->Type == InnerIter->Type)
				{
					// remove the current one
					FireflyAbilitySystemSettings->AttributeTypes.RemoveAt(Iter.GetIndex());
					--Iter;
					break;
				}
			}
		}

		bool bCreatedItem[SurfaceType_Max];
		FGenericPlatformMemory::Memzero(bCreatedItem, sizeof(bCreatedItem));

		AttributeTypeList.Empty();

		// I'm listing all of these because it is easier for users to understand how does this work. 
		// I can't just link behind of scene because internally it will only save the enum
		// for example if you name SurfaceType5 to be Water and later changed to Sand, everything that used
		// SurfaceType5 will changed to Sand
		// I think what might be better is to show what options they have, and it's for them to choose how to name

		// add the first one by default
		{
			bCreatedItem[0] = true;
			AttributeTypeList.Add(MakeShareable(new FFireflyAttributeTypeListItem(MakeShareable(new FFireflyAttributeTypeName(AttributeType_Default, TEXT("Default"))))));
		}

		// we don't create the first one. First one is always default. 
		for (auto Iter = FireflyAbilitySystemSettings->AttributeTypes.CreateIterator(); Iter; ++Iter)
		{
			bCreatedItem[Iter->Type] = true;
			AttributeTypeList.Add(MakeShareable(new FFireflyAttributeTypeListItem(MakeShareable(new FFireflyAttributeTypeName(*Iter)))));
		}

		for (int32 Index = (int32)SurfaceType1; Index < SurfaceType_Max; ++Index)
		{
			if (bCreatedItem[Index] == false)
			{
				FFireflyAttributeTypeName NeweElement((EFireflyAttributeType)Index, TEXT(""));
				AttributeTypeList.Add(MakeShareable(new FFireflyAttributeTypeListItem(MakeShareable(new FFireflyAttributeTypeName(NeweElement)))));
			}
		}

		// sort AttributeTypeList by Type

		struct FCompareAttributeType
		{
			FORCEINLINE bool operator()(const TSharedPtr<FFireflyAttributeTypeListItem> A, const TSharedPtr<FFireflyAttributeTypeListItem> B) const
			{
				check(A.IsValid());
				check(B.IsValid());
				return A->AttributeType->Type < B->AttributeType->Type;
			}
		};

		AttributeTypeList.Sort(FCompareAttributeType());

		FireflyAbilitySystemSettings->LoadSurfaceType();

		RegenerateChildren.ExecuteIfBound();
	}

	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		RegenerateChildren = InOnRegenerateChildren;
	}

	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
	{
		// no header row
	}

	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
	{
		FText SearchString = LOCTEXT("FFireflyAbilitySystemSettingsDetails_AttributeType", "Attribute Type");

		for (TSharedPtr<FFireflyAttributeTypeListItem>& Item : AttributeTypeList)
		{
			FDetailWidgetRow& Row = ChildrenBuilder.AddCustomRow(SearchString);

			FString TypeString = AttributeTypeEnum->GetNameStringByValue((int64)Item->AttributeType->Type);

			Row.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TypeString))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				];

			Row.ValueContent()
				[
					SNew(SFireflyAttributeTypeEditBox)
					.AttributeType(Item->AttributeType)
				.AttributeTypeEnum(AttributeTypeEnum)
				.OnCommitChange(this, &FAttributeTypeList::OnCommitChange)
				];
		}
	}

	virtual void Tick(float DeltaTime) override {}
	virtual bool RequiresTick() const override { return false; }
	virtual bool InitiallyCollapsed() const { return false; }
	virtual FName GetName() const override
	{
		static const FName Name(TEXT("AttributeTypeList"));
		return Name;
	}
private:

	void OnCommitChange()
	{
		bool bDoCommit = true;
		// make sure it verifies all data is correct
		// skip the first one
		for (auto Iter = AttributeTypeList.CreateConstIterator() + 1; Iter; ++Iter)
		{
			TSharedPtr<FFireflyAttributeTypeListItem> ListItem = *Iter;
			if (ListItem->AttributeType->Name != NAME_None)
			{
				// make sure no same name exists
				for (auto InnerIter = Iter + 1; InnerIter; ++InnerIter)
				{
					TSharedPtr<FFireflyAttributeTypeListItem> InnerItem = *InnerIter;
					if (ListItem->AttributeType->Name == InnerItem->AttributeType->Name)
					{
						// duplicate name, warn user and get out
						FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FFireflyAbilitySystemSettingsDetails_InvalidName", "Duplicate name found."));
						bDoCommit = false;
						break;
					}
				}
			}
		}

		if (bDoCommit)
		{
			AttributeTypesProperty->NotifyPreChange();

			FireflyAbilitySystemSettings->AttributeTypes.Empty();
			for (auto Iter = AttributeTypeList.CreateConstIterator() + 1; Iter; ++Iter)
			{
				TSharedPtr<FFireflyAttributeTypeListItem> ListItem = *Iter;
				if (ListItem->AttributeType->Name != NAME_None)
				{
					FireflyAbilitySystemSettings->AttributeTypes.Add(FFireflyAttributeTypeName(ListItem->AttributeType->Type, ListItem->AttributeType->Name));
				}
			}

			FireflyAbilitySystemSettings->TryUpdateDefaultConfigFile();
			FireflyAbilitySystemSettings->LoadSurfaceType();

			AttributeTypesProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
		}
	}
private:
	FSimpleDelegate RegenerateChildren;
	TArray< TSharedPtr< FFireflyAttributeTypeListItem > >	AttributeTypeList;
	UFireflyAbilitySystemSettings* FireflyAbilitySystemSettings;
	UEnum* AttributeTypeEnum;
	TSharedPtr<IPropertyHandle> AttributeTypesProperty;
};


//====================================================================================
// FFireflyAbilitySystemSettingsDetails
//=====================================================================================
TSharedRef<IDetailCustomization> FFireflyAbilitySystemSettingsDetails::MakeInstance()
{
	return MakeShareable(new FFireflyAbilitySystemSettingsDetails);
}

void FFireflyAbilitySystemSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& AttributeTypeCategory = DetailBuilder.EditCategory("Attribute Type", FText::GetEmpty(), ECategoryPriority::Uncommon);

	FireflyAbilitySystemSettings = UFireflyAbilitySystemSettings::Get();
	check(FireflyAbilitySystemSettings);

	FireflyAttributeTypeEnum = StaticEnum<EFireflyAttributeType>();
	check(FireflyAttributeTypeEnum);

	TSharedPtr<IPropertyHandle> AttributeTypesProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFireflyAbilitySystemSettings, AttributeTypes));

	TSharedRef<FAttributeTypeList> AttributeTypeListCustomization = MakeShareable(new FAttributeTypeList(FireflyAbilitySystemSettings, FireflyAttributeTypeEnum, AttributeTypesProperty));
	AttributeTypeListCustomization->RefreshAttributeTypeList();

	const FString AttributeTypeDocLink = TEXT("Shared/FireflyAbilitySystem");
	TSharedPtr<SToolTip> AttributeTypeTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("AttributeType", "Edit FireflyAbilitySystem's attribute types."), NULL, AttributeTypeDocLink, TEXT("AttributeType"));


	// Customize collision section
	AttributeTypeCategory.AddCustomRow(LOCTEXT("FFireflyAbilitySystemSettingsDetails_AttributeType", "Attribute Type"))
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.ToolTip(AttributeTypeTooltip)
		.AutoWrapText(true)
		.Text(LOCTEXT("AttributeType_Menu_Description", " You can have up to 62 custom surface types for your project. \nOnce you name each type, they will show up as attribute type in the blueprint editor."))
		];


	AttributeTypeCategory.AddCustomBuilder(AttributeTypeListCustomization);
}



#undef LOCTEXT_NAMESPACE


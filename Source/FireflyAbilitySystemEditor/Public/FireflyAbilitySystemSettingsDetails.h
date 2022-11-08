// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "IDetailCustomization.h"
#include "FireflyAbilitySystemSettings.h"

class IDetailLayoutBuilder;
class SEditableTextBox;
template <typename ItemType> class SListView;

DECLARE_DELEGATE(FOnCommitChange)

// Class containing the friend information - used to build the list view
class FFireflyAttributeTypeListItem
{
public:

	/**
	* Constructor takes the required details
	*/
	FFireflyAttributeTypeListItem(TSharedPtr<FFireflyAttributeTypeName> InAttributeType)
		: AttributeType(InAttributeType)
	{}

	TSharedPtr<FFireflyAttributeTypeName> AttributeType;
};

/**
* Implements the FriendsList
*/
class SFireflyAttributeTypeEditBox : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SFireflyAttributeTypeEditBox) { }
	SLATE_ARGUMENT(TSharedPtr<FFireflyAttributeTypeName>, AttributeType)
		SLATE_ARGUMENT(UEnum*, AttributeTypeEnum)
		SLATE_EVENT(FOnCommitChange, OnCommitChange)
		SLATE_END_ARGS()

public:

	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs);

	FString GetTypeString() const;

	FText GetName() const;
	void NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo);
	void OnTextChanged(const FText& NewText);

private:
	TSharedPtr<FFireflyAttributeTypeName> AttributeType;
	UEnum* AttributeTypeEnum;
	FOnCommitChange	OnCommitChange;
	TSharedPtr<SEditableTextBox> NameEditBox;
};

typedef  SListView< TSharedPtr< FFireflyAttributeTypeListItem > > SAttributeTypeListView;

class FFireflyAbilitySystemSettingsDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:

	/**
	* Generates a widget for a channel item.
	* @param InItem - the ChannelListItem
	* @param OwnerTable - the owning table
	* @return The table row widget
	*/
	TSharedRef<ITableRow> HandleGenerateListWidget(TSharedPtr< FFireflyAttributeTypeListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);


private:
	TArray< TSharedPtr< FFireflyAttributeTypeListItem > >	AttributeTypeList;

	UFireflyAbilitySystemSettings* FireflyAbilitySystemSettings;

	UEnum* FireflyAttributeTypeEnum;

	// functions
	void OnCommitChange();
};


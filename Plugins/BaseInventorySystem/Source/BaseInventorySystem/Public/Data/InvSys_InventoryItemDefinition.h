// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InvSys_InventoryItemDefinition.generated.h"

class UInvSys_InventoryItemInstance;


// Represents a fragment of an item definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UInvSys_InventoryItemInstance* Instance) const {}
};



/**
 * UGridInvSys_InventoryItemDefinition
 */
UCLASS(Blueprintable, Const, Abstract)
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UInvSys_InventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	const TArray<TObjectPtr<UInvSys_InventoryItemFragment>>& GetFragments() const;

	const UInvSys_InventoryItemFragment* FindFragmentByClass(TSubclassOf<UInvSys_InventoryItemFragment> FragmentClass) const;

	template<class T>
	const T* FindFragmentByClass() const
	{
		return (T*)FindFragmentByClass(T::StaticClass());
	}

	const FText GetItemDisplayName() const
	{
		return DisplayName;
	}

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UInvSys_InventoryItemFragment>> Fragments;
};
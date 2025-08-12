// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "InvSys_InventoryContentMapping.generated.h"

class UInvSys_InventoryLayoutWidget;

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryObjectContent : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TArray<class UInvSys_BaseInventoryFragment*> Fragments;

	template<class InventoryObjectType = UInvSys_BaseInventoryObject>
	InventoryObjectType* ConstructInventoryObject(UInvSys_InventoryComponent* InvComp, FGameplayTag InventoryObjectTag)
	{
		if (InvComp)
		{
			InventoryObjectType* InvObj = NewObject<InventoryObjectType>(InvComp);
			InvObj->InventoryObjectTag = InventoryObjectTag;
			InvObj->ConstructInventoryFragment(Fragments);
			return InvObj;
		}
		checkf(false, TEXT("InvComp is nullptr."));
		return nullptr;
	}
};

USTRUCT(BlueprintType)
struct FInvSys_InventoryObjectHelper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "InventoryObjectHelper")
	FGameplayTag InventoryObjectTag;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "InventoryObjectHelper")
	TObjectPtr<UInvSys_InventoryObjectContent> InventoryObjectContent;

	template<class InventoryObjectType = UInvSys_BaseInventoryObject>
	InventoryObjectType* ConstructInventoryObject(UInvSys_InventoryComponent* InvComp) const
	{
		if (InventoryObjectContent && InventoryObjectTag.IsValid())
		{
			return InventoryObjectContent->ConstructInventoryObject(InvComp, InventoryObjectTag);
		}
		checkf(false, TEXT("InventoryObjectContent or InventoryObjectTag is not valid."));
		return nullptr;
	}
};

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryContentMapping : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TArray<FInvSys_InventoryObjectHelper> InventoryContentMapping;
};

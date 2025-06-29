// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_CommonType.h"
#include "Engine/DataAsset.h"
#include "InvSys_InventoryItemInfo.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemInfo : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 物品ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item Info")
	FName ItemID;
	// 物品名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item Info")
	FText ItemName = FText::FromString(TEXT("默认物品"));
	// 物品描述
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item Info")
	FText ItemDescription = FText::FromString(TEXT("这是一个物品的默认描述。"));
	// 物品品质
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item Info")
	EInvSys_ItemQuality ItemQuality = EInvSys_ItemQuality::NO_0;
};

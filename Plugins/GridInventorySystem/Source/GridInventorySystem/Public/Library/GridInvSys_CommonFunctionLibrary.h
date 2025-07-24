// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GridInvSys_CommonFunctionLibrary.generated.h"

class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_CommonFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 计算物品实例经过旋转后的大小 */
	UFUNCTION(BlueprintPure, Category = "Common Function Library")
	static FIntPoint CalculateItemInstanceSize(UInvSys_InventoryItemInstance* ItemInstance);

	/** 计算物品实例经过旋转后的大小 */
	UFUNCTION(BlueprintPure, Category = "Common Function Library")
	static FIntPoint CalculateItemInstanceSizeFrom(UInvSys_InventoryItemInstance* ItemInstance,
		EGridInvSys_ItemDirection ItemDirection);

	UFUNCTION(BlueprintPure, Category = "Common Function Library")
	static FIntPoint CalculateItemDefinitionSize(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef);
	
	/** 计算物品实例经过旋转后的大小 */
	UFUNCTION(BlueprintPure, Category = "Common Function Library")
	static FIntPoint CalculateItemDefinitionSizeFrom(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef,
		EGridInvSys_ItemDirection ItemDirection);
};

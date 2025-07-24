// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/GridInvSys_CommonFunctionLibrary.h"

#include "GridInvSys_CommonType.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Data/InvSys_InventoryItemInstance.h"

FIntPoint UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSize(UInvSys_InventoryItemInstance* ItemInstance)
{
	const UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
	if (GridItemInstance)
	{
		return CalculateItemInstanceSizeFrom(ItemInstance, GridItemInstance->GetItemPosition().Direction);
	}
	return FIntPoint(1, 1);
}

FIntPoint UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(UInvSys_InventoryItemInstance* ItemInstance,
	EGridInvSys_ItemDirection ItemDirection)
{
	const UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
	if (GridItemInstance == nullptr)
	{
		return FIntPoint(1, 1);
	}
	
	FIntPoint NativeItemSize = FIntPoint(1, 1);
	FIntPoint TargetItemSize = NativeItemSize;
	// 根据方向计算物体实际大小
	if (auto ItemSizeFragment = GridItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>())
	{
		NativeItemSize = ItemSizeFragment->ItemSize;
	}
	else
	{
		checkNoEntry();
		return NativeItemSize;
	}
	// 计算旋转后的物品大小
	switch (ItemDirection)
	{
	case EGridInvSys_ItemDirection::Horizontal:
		TargetItemSize = NativeItemSize;
		break;
	case EGridInvSys_ItemDirection::Vertical:
		TargetItemSize.X = NativeItemSize.Y;
		TargetItemSize.Y = NativeItemSize.X;
		break;
	}
	return TargetItemSize;
}

FIntPoint UGridInvSys_CommonFunctionLibrary::CalculateItemDefinitionSize(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef)
{
	if (ItemDef)
	{
		return CalculateItemDefinitionSizeFrom(ItemDef, EGridInvSys_ItemDirection::Horizontal);
	}
	return FIntPoint(1, 1);
}

FIntPoint UGridInvSys_CommonFunctionLibrary::CalculateItemDefinitionSizeFrom(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, EGridInvSys_ItemDirection ItemDirection)
{
	
	if (ItemDef == nullptr)
	{
		return FIntPoint(1, 1);
	}
	auto ItemDefObj = ItemDef->GetDefaultObject<UInvSys_InventoryItemDefinition>();
	
	FIntPoint NativeItemSize = FIntPoint(1, 1);
	FIntPoint TargetItemSize = NativeItemSize;
	// 根据方向计算物体实际大小
	if (auto ItemSizeFragment = ItemDefObj->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>())
	{
		NativeItemSize = ItemSizeFragment->ItemSize;
	}
	else
	{
		checkNoEntry();
		return NativeItemSize;
	}
	// 计算旋转后的物品大小
	switch (ItemDirection)
	{
	case EGridInvSys_ItemDirection::Horizontal:
		TargetItemSize = NativeItemSize;
		break;
	case EGridInvSys_ItemDirection::Vertical:
		TargetItemSize.X = NativeItemSize.Y;
		TargetItemSize.Y = NativeItemSize.X;
		break;
	}
	return TargetItemSize;
}

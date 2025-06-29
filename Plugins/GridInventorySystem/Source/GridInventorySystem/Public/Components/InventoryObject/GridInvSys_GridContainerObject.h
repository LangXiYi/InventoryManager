// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InventoryObject/InvSys_BaseContainerObject.h"
#include "GridInvSys_GridContainerObject.generated.h"


/**
 * 会为库存组件添加固定样式的容器
 * ============
 * 纯容器
 * ============
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_GridContainerObject : public UInvSys_BaseContainerObject
{
	GENERATED_BODY()
	
public:
	virtual void InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad) override;

	virtual void AddInventoryItemToContainer(const FGridInvSys_InventoryItem& NewItem);
	
	virtual void CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad) override;

	virtual void CreateDisplayWidget(APlayerController* PC) override;
	
	/**
	 * 什么情况下 AddInventoryItemTo 会被调用？
	 * 1、程序直接调用
	 * 2、从其他库存组件添加物品到该库存组件
	 *		单个
	 *		多个
	 * @param ItemUniqueID 
	 * @param NewItem 
	 */
	virtual void AddInventoryItemToContainer(FName ItemUniqueID, FGridInvSys_InventoryItem NewItem);

	virtual void RemoveInventoryItemFromContainer(FName ItemUniqueID);

	FORCEINLINE UUserWidget* GetContainerGridLayout() const
	{
		return ContainerGridLayoutWidget;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	/** 容器网格布局 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Type")
	TSubclassOf<UUserWidget> ContainerGridLayoutWidgetClass;

	/** 容器网格布局 */
	UPROPERTY(BlueprintReadOnly, Category = "Container Type")
	TObjectPtr<UUserWidget> ContainerGridLayoutWidget;

	// <GridID, ContainerGridWidgets> 供客户端使用
	UPROPERTY(BlueprintReadOnly, Category = "Container Type")
	TMap<FName, UUserWidget*> ContainerGridMap;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=OnRep_InventoryItems, Category = "Inventory Component")
	TArray<FGridInvSys_InventoryItem> InventoryItems;
	UFUNCTION()
	void OnRep_InventoryItems();

private:
	/** 加快物品的查询速度。 Key ===> ItemUniqueID */
	TMap<FName, FGridInvSys_InventoryItem> InventoryItemMap;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_PreEditGridContainerObject : public UInvSys_PreEditInventoryObject
{
	GENERATED_BODY()
public:
	CONSTRUCT_INVENTORY_OBJECT(UGridInvSys_GridContainerObject);
	
	/** 容器网格布局 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Type")
	TSubclassOf<UUserWidget> ContainerGridLayoutWidgetClass;
};
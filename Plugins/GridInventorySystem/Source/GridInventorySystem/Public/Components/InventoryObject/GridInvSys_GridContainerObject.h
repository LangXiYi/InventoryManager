// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InventoryObject/InvSys_BaseContainerObject.h"
#include "GridInvSys_GridContainerObject.generated.h"


class UGridInvSys_ContainerGridWidget;
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
	UGridInvSys_GridContainerObject();
	
	virtual void InitInventoryObject(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad) override;

	virtual void AddInventoryItemToContainer(const FGridInvSys_InventoryItem& InventoryItem);
	virtual void RemoveInventoryItemFromContainer(FGridInvSys_InventoryItem InventoryItem);
	virtual void UpdateInventoryItemFromContainer(FGridInvSys_InventoryItem NewItem);

protected:
	virtual void CreateDisplayWidget(APlayerController* PC) override;

	virtual void OnAddedContainerItems(const TArray<FName>& InAddedItems) override {}
	virtual void OnRemovedContainerItems(const TArray<FName>& InRemovedItems) override {}
	virtual void OnUpdatedContainerItems(const TArray<FName>& InChangedItems) override {}
	
public:
	/**
	 * Getter Or Setter
	 **/

	bool FindContainerGridItem(const FIntPoint& ItemPosition, FGridInvSys_InventoryItem& OutItem) const;

	bool FindContainerGridItem(FName ItemUniqueID, FGridInvSys_InventoryItem& OutItem) const;
	
	FORCEINLINE UUserWidget* GetContainerGridLayout() const
	{
		return ContainerGridLayoutWidget;
	}

	virtual void CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad) override;
	
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
	/** Server: 优化物品查询速度  Key ===> Position : Value ===> ItemUniqueId */
	TMap<FIntPoint, FName> ItemPositionMap;
	/** Server & Client: 加快物品的查询速度。 Key ===> ItemUniqueID */
	TMap<FName, FGridInvSys_InventoryItem> ContainerGridItems;
	// Client: <GridID, ContainerGridWidgets> 供客户端使用，会在创建控件之后自动填充。
	TMap<FName, UGridInvSys_ContainerGridWidget*> ContainerGridWidgets;
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
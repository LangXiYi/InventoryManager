// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_EquipmentSlotWidget.h"
#include "GridInvSys_EquipContainerSlotWidget.generated.h"

class UGridInvSys_ContainerGridWidget;
struct FGridInvSys_InventoryItem;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_EquipContainerSlotWidget : public UGridInvSys_EquipmentSlotWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateOccupant(const FInvSys_InventoryItem& NewOccupant) override;

	virtual void UpdateContainerGrid(const TArray<FGridInvSys_InventoryItem>& AllItems);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateContainerGrid(const TArray<FGridInvSys_InventoryItem>& AllItems);

public:
	// 获取容器布局对象
	UGridInvSys_ContainerGridLayoutWidget* GetContainerGridLayoutWidget();

	void GetAllContainerGrid(TArray<UGridInvSys_ContainerGridWidget*>& OutContainerGrids);

protected:
	UPROPERTY()
	TObjectPtr<UGridInvSys_ContainerGridLayoutWidget> ContainerLayoutWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UNamedSlot> NS_ContainerGridLayout;
};

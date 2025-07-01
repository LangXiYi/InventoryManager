// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_EquipContainerSlotWidget.h"

#include "InvSys_CommonType.h"
#include "Components/NamedSlot.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"
#include "Data/GridInvSys_InventoryContainerInfo.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"


void UGridInvSys_EquipContainerSlotWidget::UpdateOccupant(const FInvSys_InventoryItem& NewOccupant)
{
	if (NewOccupant.ItemInfo.IsA(UGridInvSys_InventoryContainerInfo::StaticClass()) == false)
	{
		return;
	}
	// 根据Occupant创建容器布局控件
	if (UGridInvSys_InventoryContainerInfo* ContainerInfo = Cast<UGridInvSys_InventoryContainerInfo>(NewOccupant.ItemInfo))
	{
		if (ContainerInfo->ContainerGridLayoutWidgetClass)
		{
			// todo:: remove old widget.
			ContainerLayoutWidget = CreateWidget<UGridInvSys_ContainerGridLayoutWidget>(
				GetOwningPlayer(), ContainerInfo->ContainerGridLayoutWidgetClass);
			// 构建容器网格
			ContainerLayoutWidget->SetInventoryComponent(GetInventoryComponent());
			ContainerLayoutWidget->ConstructContainerGrid(GetSlotName());
			NS_ContainerGridLayout->AddChild(ContainerLayoutWidget);
		}
	}
	Super::UpdateOccupant(NewOccupant);
}

void UGridInvSys_EquipContainerSlotWidget::UpdateContainerGrid(const TArray<FGridInvSys_InventoryItem>& AllItems)
{
	TArray<UGridInvSys_ContainerGridWidget*> ContainerGridWidgets;
	ContainerLayoutWidget->GetAllContainerGridWidgets(ContainerGridWidgets);
	for (UGridInvSys_ContainerGridWidget* ContainerGridWidget : ContainerGridWidgets)
	{
		// ContainerGridWidget->ClearAll();
	}
	
	//获取Layout下
	for (const FGridInvSys_InventoryItem& Item : AllItems)
	{
		UGridInvSys_ContainerGridWidget* GridWidget = ContainerLayoutWidget->FindContainerGrid(Item.ItemPosition.GridID);
		if (GridWidget)
		{
			GridWidget->AddInventoryItemTo(Item);
		}
	}
}

UGridInvSys_ContainerGridLayoutWidget* UGridInvSys_EquipContainerSlotWidget::GetContainerGridLayoutWidget()
{
	return ContainerLayoutWidget;
}

void UGridInvSys_EquipContainerSlotWidget::GetAllContainerGrid(
	TArray<UGridInvSys_ContainerGridWidget*>& OutContainerGrids)
{
	 ContainerLayoutWidget->GetAllContainerGridWidgets(OutContainerGrids);
}

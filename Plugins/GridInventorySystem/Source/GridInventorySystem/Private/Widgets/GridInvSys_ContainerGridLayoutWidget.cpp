// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"

#include "BaseInventorySystem.h"
#include "Components/PanelWidget.h"
#include "Data/GridInvSys_InventoryContainerInfo.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"

#if WITH_EDITOR
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#endif

void UGridInvSys_ContainerGridLayoutWidget::ConstructContainerGrid(FName SlotName)
{
	/*if (ContainerInfo == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("[%s] 控件未设置 Container Info。"), *GetName())
		return;
	}
	// 初始化容器网格
	ContainerGridWidgets.Empty();
	GetAllContainerGridWidgets(ContainerGridWidgets);
	ContainerGridMap.Empty();
	ContainerGridMap.Reserve(ContainerGridWidgets.Num());
	for (int i = 0; i < ContainerGridWidgets.Num(); ++i)
	{
		FName GridID = *FString::FromInt(i);
		ContainerGridWidgets[i]->SetContainerGridID(GridID);
		
		ContainerGridWidgets[i]->UpdateContainerGridSize();

		ContainerGridWidgets[i]->SetInventoryComponent(GetInventoryComponent());
		ContainerGridWidgets[i]->ConstructGridItems(SlotName);
		ContainerGridMap.Add(ContainerGridWidgets[i]->GetContainerGridID(), ContainerGridWidgets[i]);
	}*/
}

UGridInvSys_ContainerGridWidget* UGridInvSys_ContainerGridLayoutWidget::FindContainerGrid(FName GridID)
{
	if (ContainerGridMap.Contains(GridID))
	{
		return ContainerGridMap[GridID];
	}
	return nullptr;
}

void UGridInvSys_ContainerGridLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ContainerGridWidgets.Empty();
	GetAllContainerGridWidgets(ContainerGridWidgets);
	for (int i = 0; i < ContainerGridWidgets.Num(); ++i)
	{
		ContainerGridWidgets[i]->ConstructGridItems(this, i);
	}
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridLayoutWidget::FindGridItemWidget(const FGridInvSys_ItemPosition& ItemPosition) const
{
	if (SlotTag == ItemPosition.EquipSlotTag)
	{
		if (UGridInvSys_ContainerGridWidget* GridWidget = ContainerGridWidgets[ItemPosition.GridID])
		{
			return GridWidget->GetGridItemWidget(ItemPosition.Position);
		}
	}
	return nullptr;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_ContainerGridLayoutWidget::FindGridItemWidget(
	const UInvSys_InventoryItemInstance* InItemInstance) const
{
	if (InItemInstance && InItemInstance->IsA(UGridInvSys_InventoryItemInstance::StaticClass()))
	{
		UGridInvSys_InventoryItemInstance* TempItemInstance = (UGridInvSys_InventoryItemInstance*)InItemInstance;
		check(TempItemInstance);
		return FindGridItemWidget(TempItemInstance->GetItemPosition());
	}
	return nullptr;
}

void UGridInvSys_ContainerGridLayoutWidget::GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray) const
{
	//OutArray.Empty();
	UWidget* RootWidget = GetRootWidget();
	check(RootWidget);
	Private_GetAllContainerGridWidgets(OutArray, RootWidget);
}

void UGridInvSys_ContainerGridLayoutWidget::Private_GetAllContainerGridWidgets(
	TArray<UGridInvSys_ContainerGridWidget*>& OutArray, UWidget* Parent) const
{
	if (Parent == nullptr)
	{
		return;
	}

	// 递归结束条件：Parent 不为 PanelWidget 类型。
	UPanelWidget* PanelWidget = Cast<UPanelWidget>(Parent);
	if (PanelWidget == nullptr)
	{
		return;
	}
	
	TArray<UWidget*> AllChildren = PanelWidget->GetAllChildren();
	for (UWidget* ChildWidget : AllChildren)
	{
		if (UGridInvSys_ContainerGridWidget* GridWidget = Cast<UGridInvSys_ContainerGridWidget>(ChildWidget))
		{
			OutArray.Add(GridWidget);
			continue;
		}
		// 递归查找子集中是否存在 UContainerGridWidget
		Private_GetAllContainerGridWidgets(OutArray, ChildWidget);
	}
}

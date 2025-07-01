// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"

#include "Components/PanelWidget.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"

#if WITH_EDITOR
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#endif

void UGridInvSys_ContainerGridLayoutWidget::ConstructContainerGrid(FName SlotName)
{
	// 初始化容器网格
	GetAllContainerGridWidgets(ContainerGridWidgets);
	ContainerGridMap.Empty();
	ContainerGridMap.Reserve(ContainerGridWidgets.Num());
	for (int i = 0; i < ContainerGridWidgets.Num(); ++i)
	{
		ContainerGridWidgets[i]->SetContainerGridID(*FString::FromInt(i));
		ContainerGridWidgets[i]->SetInventoryComponent(GetInventoryComponent());
		ContainerGridWidgets[i]->ConstructGridItems(SlotName);
		ContainerGridMap.Add(ContainerGridWidgets[i]->GetContainerGridID(), ContainerGridWidgets[i]);
	}
}

UGridInvSys_ContainerGridWidget* UGridInvSys_ContainerGridLayoutWidget::FindContainerGrid(FName GridID)
{
	if (ContainerGridMap.Contains(GridID))
	{
		return ContainerGridMap[GridID];
	}
	return nullptr;
}

void UGridInvSys_ContainerGridLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 自动生成 ContainerGridID

}

void UGridInvSys_ContainerGridLayoutWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		GetAllContainerGridWidgets(ContainerGridWidgets);
		for (int i = 0; i < ContainerGridWidgets.Num(); ++i)
		{
			ContainerGridWidgets[i]->SetContainerGridID(*FString::FromInt(i));
		}
		// ConstructContainerGrid(NAME_None);
#ifdef WITH_EDITOR
		// 判断内部是否存在重读的 ContainerGridID
		TSet<FName> TempSet;
		TArray<UGridInvSys_ContainerGridWidget*> OutArray;
		GetAllContainerGridWidgets(OutArray);
		for (UGridInvSys_ContainerGridWidget* ContainerGridWidget : OutArray)
		{
			FName GridID = ContainerGridWidget->GetContainerGridID();
			if (TempSet.Contains(GridID))
			{
				FNotificationInfo Info(FText::FromString(TEXT("布局内存在重复的 Contianer Grid ID ==> " + GridID.ToString())));
				Info.ExpireDuration = 3.f;
				FSlateNotificationManager::Get().AddNotification(Info);
				continue;
			}
			TempSet.Add(GridID);
		}
#endif
	}
}

void UGridInvSys_ContainerGridLayoutWidget::GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray)
{
	OutArray.Empty();
	Private_GetAllContainerGridWidgets(OutArray, GetRootWidget());
}

void UGridInvSys_ContainerGridLayoutWidget::Private_GetAllContainerGridWidgets(TArray<UGridInvSys_ContainerGridWidget*>& OutArray, UWidget* Parent)
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

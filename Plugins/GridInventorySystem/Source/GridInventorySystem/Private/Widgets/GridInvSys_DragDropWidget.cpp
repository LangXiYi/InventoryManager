// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_DragDropWidget.h"

#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include"Components/GridInvSys_InventoryComponent.h"
#include "Components/GridInvSys_GridInventoryControllerComponent.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_DragItemWidget.h"
#include "Data/InvSys_InventoryItemInfo.h"


#if WITH_EDITOR
#include "Widgets/Notifications/SNotificationList.h"
#endif

UGridInvSys_DragDropWidget::UGridInvSys_DragDropWidget(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	if (bIsOverrideDraggingWidgetClass == false)
	{
		if (const UGridInvSys_InventorySystemConfig* InventorySystemConfig = GetDefault<UGridInvSys_InventorySystemConfig>())
		{
			check(InventorySystemConfig->DraggingWidgetClass)
			DraggingWidgetClass = InventorySystemConfig->DraggingWidgetClass;
		}
	}
	
	DragPivot = EDragPivot::CenterCenter;
	DragOffset = FVector2D(0.f, 0.f);
	SetVisibilityInternal(ESlateVisibility::Visible);
}

void UGridInvSys_DragDropWidget::UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo)
{
	ItemInfo = NewItemInfo;
}

void UGridInvSys_DragDropWidget::SetGridItemWidget(UGridInvSys_ContainerGridItemWidget* NewGridItemWidget)
{
	GridItemWidget = NewGridItemWidget;
}

UGridInvSys_ContainerGridItemWidget* UGridInvSys_DragDropWidget::GetGridItemWidget() const
{
	return GridItemWidget;
}

EDragDropType UGridInvSys_DragDropWidget::GetDragDropType() const
{
	return DragDropType;
}

void UGridInvSys_DragDropWidget::SetDraggingWidgetClass(TSubclassOf<UGridInvSys_DragItemWidget> NewDraggingWidgetClass)
{
	DraggingWidgetClass = NewDraggingWidgetClass;
}

FReply UGridInvSys_DragDropWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (ItemInfo)
	{
		FEventReply EventReply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
		return EventReply.NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UGridInvSys_DragDropWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	// TODO: 创建拖拽对象。

	// 创建 Dragging 控件
	UGridInvSys_DragItemWidget* DraggingWidget = CreateWidget<UGridInvSys_DragItemWidget>(GetOwningPlayer(), DraggingWidgetClass);
	DraggingWidget->UpdateItemInfo(ItemInfo);
	
	UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>();
	DragDropOperation->Tag = DragDropTypeString[DragDropType];
	DragDropOperation->Payload = this;
	DragDropOperation->DefaultDragVisual = DraggingWidget;
	DragDropOperation->Pivot = DragPivot;
	DragDropOperation->Offset = DragOffset;
	OutOperation = DragDropOperation;
}

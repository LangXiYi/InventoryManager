// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_DragDropWidget.h"

#include "GridInvSys_InventorySystemConfig.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/GridInvSys_DraggingWidget.h"
#include "Data/InvSys_InventoryItemInfo.h"


#if WITH_EDITOR
#include "Widgets/Notifications/SNotificationList.h"
#endif

UGridInvSys_DragDropWidget::UGridInvSys_DragDropWidget(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	if (bOverrideAccessibleDefaults == false)
	{
		const UGridInvSys_InventorySystemConfig* InventorySystemConfig = GetDefault<UGridInvSys_InventorySystemConfig>();
#if WITH_EDITOR
		if (IsValid(InventorySystemConfig->DraggingWidgetClass) == false)
		{
			FNotificationInfo Info(FText::FromString(TEXT("DraggingWidgetClass is nullptr, 请在前往更新项目设置->库存系统->DraggingWidgetClass")));	
			Info.ExpireDuration = 4.f;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
#endif
		DraggingWidgetClass = InventorySystemConfig->DraggingWidgetClass;
	}
	DragPivot = EDragPivot::CenterCenter;
	DragOffset = FVector2D(0.f, 0.f);
	SetVisibilityInternal(ESlateVisibility::Visible);
}

void UGridInvSys_DragDropWidget::UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo)
{
	ItemInfo = NewItemInfo;
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
	UGridInvSys_DraggingWidget* DraggingWidget = CreateWidget<UGridInvSys_DraggingWidget>(GetOwningPlayer(), DraggingWidgetClass);
	DraggingWidget->UpdateItemInfo(ItemInfo);
	
	UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>();
	DragDropOperation->Payload = ItemInfo;
	DragDropOperation->DefaultDragVisual = DraggingWidget;
	DragDropOperation->Pivot = DragPivot;
	DragDropOperation->Offset = DragOffset;
	OutOperation = DragDropOperation;
}

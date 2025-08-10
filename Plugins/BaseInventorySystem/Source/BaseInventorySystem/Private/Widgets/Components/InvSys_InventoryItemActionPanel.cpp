// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/InvSys_InventoryItemActionPanel.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Data/InvSys_InventoryItemInstance.h"

void UInvSys_InventoryItemActionPanel::NativePreConstruct()
{
	Super::NativePreConstruct();
	// if (IsDesignTime())
	// {
	// 	ActionList->RemoveFromParent();
	// 	Canvas->AddChildToCanvas(ActionList);
	// }
}

void UInvSys_InventoryItemActionPanel::NativeConstruct()
{
	Super::NativeConstruct();
	ActionList->SetVisibility(ESlateVisibility::Visible);
	SetVisibility(ESlateVisibility::Hidden);
}

void UInvSys_InventoryItemActionPanel::NativeOnCallOut(UInvSys_InventoryItemInstance* InItemInstance)
{
	ItemInstance = InItemInstance;
	SetVisibility(ESlateVisibility::Visible);

	// 更新列表位置
	UCanvasPanelSlot* ActionListSlot= UWidgetLayoutLibrary::SlotAsCanvasSlot(ActionList);
	FVector2D MousePositionOnViewport = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
	ActionListSlot->SetPosition(MousePositionOnViewport);

	OnCallOut(InItemInstance);
}

void UInvSys_InventoryItemActionPanel::NativeOnCallBack()
{
	// 将该操作列表隐藏
	SetVisibility(ESlateVisibility::Collapsed);
	ItemInstance = nullptr;
	OnCallBack();
}

FReply UInvSys_InventoryItemActionPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	NativeOnCallBack();
	return FReply::Handled();
}

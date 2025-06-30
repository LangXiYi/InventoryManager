// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GridInvSys_ContainerGridDropWidget.h"

#include "GridInvSys_InventorySystemConfig.h"
#include "Components/SizeBox.h"

void UGridInvSys_ContainerGridDropWidget::NativeOnDraggingHovered(bool bIsCanDrop)
{
	SetVisibility(ESlateVisibility::Visible);
	OnDraggingHovered(bIsCanDrop);
}

void UGridInvSys_ContainerGridDropWidget::NativeOnEndDraggingHovered()
{
	SetVisibility(ESlateVisibility::Hidden);
	OnEndDraggingHovered();
}

void UGridInvSys_ContainerGridDropWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		SetVisibility(ESlateVisibility::Visible);
		if (const UGridInvSys_InventorySystemConfig* SystemConfig = GetDefault<UGridInvSys_InventorySystemConfig>())
		{
			DraggingDisplayGridItem->SetHeightOverride(SystemConfig->ItemDrawSize);
			DraggingDisplayGridItem->SetWidthOverride(SystemConfig->ItemDrawSize);
		}
	}
}

void UGridInvSys_ContainerGridDropWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (const UGridInvSys_InventorySystemConfig* SystemConfig = GetDefault<UGridInvSys_InventorySystemConfig>())
	{
		DraggingDisplayGridItem->SetHeightOverride(SystemConfig->ItemDrawSize);
		DraggingDisplayGridItem->SetWidthOverride(SystemConfig->ItemDrawSize);
	}
	SetVisibility(ESlateVisibility::Hidden);
}

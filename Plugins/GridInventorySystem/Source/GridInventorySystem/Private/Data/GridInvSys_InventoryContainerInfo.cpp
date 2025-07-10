// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GridInvSys_InventoryContainerInfo.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"

#include "Blueprint/UserWidget.h"
#include "Widgets/GridInvSys_ContainerGridWidget.h"

void UGridInvSys_InventoryContainerInfo::GetAllContainerGridSize()
{
	// ContainerGridLayoutWidgetClass
	
	const UGridInvSys_ContainerGridLayoutWidget* TempObject = GetDefault<UGridInvSys_ContainerGridLayoutWidget>(ContainerGridLayoutWidgetClass);
	check(TempObject);
	TArray<UGridInvSys_ContainerGridWidget*> OutArray = TempObject->GetContainerGridWidgets();
	for (const UGridInvSys_ContainerGridWidget* Item : OutArray)
	{
		// ContainerGridSizeMap.Add(Item->GetContainerGridID(), Item->GetContainerGridSize());
	}
}

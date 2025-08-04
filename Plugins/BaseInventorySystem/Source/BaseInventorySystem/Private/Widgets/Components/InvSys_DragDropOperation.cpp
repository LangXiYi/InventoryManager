// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/InvSys_DragDropOperation.h"

void UInvSys_DragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(3, 2, FColor::Red, TEXT("Drop"));
	}
	Super::Drop_Implementation(PointerEvent);
}

void UInvSys_DragDropOperation::Dragged_Implementation(const FPointerEvent& PointerEvent)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 2, FColor::Green, TEXT("Dragged"));
	}
	Super::Dragged_Implementation(PointerEvent);
}

void UInvSys_DragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 2, FColor::Red, TEXT("DragCancelled"));
	}
	Super::DragCancelled_Implementation(PointerEvent);
}

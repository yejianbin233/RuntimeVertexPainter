// Fill out your copyright notice in the Description page of Project Settings.


#include "VertexPainterLibrary.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(VertexPainterBlueprintFunctionLibraryLog)

void UVertexPainterLibrary::PaintVertexColorByIndex(UStaticMeshComponent* StaticMeshComponent, FLinearColor Color, int32 Index, int32 LODIndex)
{
	if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
	{
		if (!StaticMeshComponent)
		{
			UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("静态网格体组件无效!"));
		}
		else if (!StaticMeshComponent->GetStaticMesh())
		{
			UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("静态网格体组件未设置静态网格模型!"));
		}
		return;
	}

	// 如果不进行这一系列处理，无法通过判断，LODIndex 始终是无效的。
	const int32 LODNum = StaticMeshComponent->GetStaticMesh()->GetNumLODs();
	StaticMeshComponent->SetLODDataCount(LODNum, LODNum);

	// 判断 LOD Index 是否是有效 LOD 索引
	if (!StaticMeshComponent->LODData.IsValidIndex(LODIndex))
	{
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("LOD Index 无效，请输入正确的 LOD Index!"));
		return;
	}
	// ===

	// 从静态网格体组件中获取指定 LOD Index 的信息
	FStaticMeshComponentLODInfo& LODInfo = StaticMeshComponent->LODData[LODIndex];

	// 获取渲染数据中的对应的 LOD 数据源，在获取顶点数量
	const int32 VertexNum = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndex].GetNumVertices();

	if (Index < 0 || Index >= VertexNum)
	{
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("Index 越界无效，请输入正确的 Index!"));
		return;
	}

	// 颜色数组
	TArray<FColor> VertexColors;
	VertexColors.SetNum(VertexNum);

	// 如果没有对网格体的 "OverrideVertexColors" 数组进行颜色冲刷，那么可能获取到空的数据数组，因此如果 "VertexColor" 的数据为空，则需要进行颜色冲刷
	if (LODInfo.OverrideVertexColors)
	{
		// 先读取元素的顶点颜色数组
		LODInfo.OverrideVertexColors->GetVertexColors(VertexColors);
	}
	else
	{
		// 判断是否有顶点颜色
		if (StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndex].bHasColorVertexData)
		{
			// 有顶点颜色
			StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndex].VertexBuffers.ColorVertexBuffer.GetVertexColors(VertexColors);
		} else
		{
			// 没有，默认顶点颜色为白色
			VertexColors.Init(FColor::White, VertexNum);
			UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("网格体重载顶点颜色未设置，默认初始化为白色。"));
		}
	}
	// 再将覆盖指定索引的顶点颜色
	VertexColors[Index] = Color.ToFColor(true);
	
	// 创建 FColorVertexBuffer 指针，省略此步骤会报错
	LODInfo.OverrideVertexColors = new FColorVertexBuffer;
	LODInfo.OverrideVertexColors->InitFromColorArray(VertexColors);

	// 通知重新初始化
	// note：如果想要顶点颜色正确显示在网格体模型上，那么应该创建一个材质，并在材质上使用"Vertex Color"节点获取顶点颜色，并连接到"基础颜色"上，"自发光"不能正确显示顶点颜色，必须连接"基础颜色"。
	BeginInitResource(LODInfo.OverrideVertexColors);

	// 将渲染状态标记为脏 - 将在帧结束时发送到渲染线程。
	StaticMeshComponent->MarkRenderStateDirty();
	
	// 如果为 true，则不允许在此实例上进行网格绘制。设置是否在构造脚本中覆盖顶点颜色。
	StaticMeshComponent->bDisallowMeshPaintPerInstance = true;
}

void UVertexPainterLibrary::PaintVertexColorInSphere(UStaticMeshComponent* StaticMeshComponent, FLinearColor Color, float Radius, FVector Position)
{
	// // 教程的这一步直接不参照着实现了。
	// const int32 LODNum = StaticMeshComponent->GetStaticMesh()->GetNumLODs();
	// StaticMeshComponent->SetLODDataCount(LODNum, LODNum);
	//
	// // 先将 Position 从世界坐标转换为 StaticMeshComponent 的本地位置，便于与顶点位置进行比较
	// FTransform StaticMeshTransform = StaticMeshComponent->GetComponentTransform();
	//
	// FVector PositionToStaticMeshLocation = UKismetMathLibrary::InverseTransformLocation(StaticMeshTransform, Position);
	//
	// // 遍历每个 LOD 数据
	// for (FStaticMeshComponentLODInfo LODInfo:StaticMeshComponent->LODData)
	// {
	// 	// 从同一 LOD 数据，从 0 开始遍历
	// 	int32 LODIndex = 0;
	// 	
	// 	// 获取网格体所有顶点的位置信息
	// 	const FPositionVertexBuffer VertexPositionBuffer = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndex].VertexBuffers.PositionVertexBuffer;
	//
	// 	// 获取顶点数量
	// 	int VertexNum = VertexPositionBuffer.GetNumVertices();
	//
	// 	// 遍历每个顶点
	// 	for(int32 VertexIndex = 0; VertexIndex < VertexNum; VertexIndex++)
	// 	{
	// 		// 判断顶点位置是否在被绘制区域
	// 		FVector VertexPosition = static_cast<UE::Math::TVector4<double>>(VertexPositionBuffer.VertexPosition(VertexIndex));
	//
	// 		float Distance = UKismetMathLibrary::Vector_Distance(VertexPosition, PositionToStaticMeshLocation);
	// 		
	// 		float NormalizedDistance = Distance / Radius;
	//
	// 		
	// 		if (NormalizedDistance <= 1)
	// 		{
	// 			// 在绘制范围内，收集在绘制球体范围内的顶点
	// 			
	// 			// TODO - 然后绘制
	// 		}
	// 	}
	// }
}

TArray<FColor> UVertexPainterLibrary::GetStaticMeshVertexColors(UStaticMeshComponent* StaticMeshComponent, int32 LODIndex)
{
	TArray<FColor> VertexColors;
	if (!StaticMeshComponent)
	{
		// 无效静态网格体组件指针
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，静态网格体组件指针无效!"))
		return VertexColors;
	}
	
	if (!StaticMeshComponent->GetStaticMesh())
	{
		// 无效静态网格体
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，未设置静态网格体!"))
		return VertexColors;
	}

	const int32 LODNum = StaticMeshComponent->GetStaticMesh()->GetNumLODs();
	StaticMeshComponent->SetLODDataCount(LODNum, LODNum);
	
	if (!StaticMeshComponent->LODData.IsValidIndex(LODIndex))
	{
		// 无效 LOD 索引
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，LOD Index 无效!"))
		return VertexColors;
	}
	
	// 顶点数量
	const int32 VertexNum = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndex].GetNumVertices();
	// VertexColors.SetNum(VertexNum); // 如果不使用 Init 设置默认颜色，那么渲染时会自动提供顶点颜色(在我的实例中是自动填充的是蓝色、黑色)
	// 设置默认颜色及数组大小
	VertexColors.Init(FColor::White, VertexNum);
	
	FStaticMeshComponentLODInfo& LODInfo = StaticMeshComponent->LODData[LODIndex];
	
	// 必须进行判断，否则如果网格体没有顶点颜色(OverrideVertexColors)则会报错
	if (LODInfo.OverrideVertexColors)
	{
		LODInfo.OverrideVertexColors->GetVertexColors(VertexColors);
	}
	return VertexColors;
}

TArray<FColor> UVertexPainterLibrary::GetSkeletalMeshVertexColors(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex)
{
	TArray<FColor> VertexColors;
	if (!SkeletalMeshComponent)
	{
		// 无效骨骼网格体组件指针
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，骨骼网格体组件指针无效!"))
		return VertexColors;
	}
	
	if (!SkeletalMeshComponent->SkeletalMesh)
	{
		// 无效骨骼网格体
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，未设置骨骼网格体!"))
		return VertexColors;
	}

	// const int32 LODNum = StaticMeshComponent->GetStaticMesh()->GetNumLODs();
	// StaticMeshComponent->SetLODDataCount(LODNum, LODNum);
	if (!SkeletalMeshComponent->LODInfo.IsValidIndex(LODIndex))
	{
		// 无效 LOD 索引
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，LOD Index 无效!"))
		return VertexColors;
	}
	
	// 顶点数量
	const int32 VertexNum = SkeletalMeshComponent->SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex].GetNumVertices();
	// 设置默认颜色及数组大小
	VertexColors.Init(FColor::White, VertexNum);
	FSkelMeshComponentLODInfo& LODInfo = SkeletalMeshComponent->LODInfo[LODIndex];
	
	// 先读取元素的顶点颜色数组
	// 必须进行判断，否则如果网格体没有顶点颜色(OverrideVertexColors)则会报错
	if (LODInfo.OverrideVertexColors)
	{
		LODInfo.OverrideVertexColors->GetVertexColors(VertexColors);
	}

	return VertexColors;
}

void UVertexPainterLibrary::OverrideStaticMeshVertexColor(UStaticMeshComponent* StaticMeshComponent, int32 LODIndex, TArray<FVertexOverrideColorInfo> VertexOverrideColorInfos)
{
	if (!StaticMeshComponent)
	{
		// 无效静态网格体组件指针
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("覆盖顶点颜色失败，静态网格体组件指针无效!"))
		return;
	}

	if (!StaticMeshComponent->GetStaticMesh())
	{
		// 无效静态网格体
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，未设置静态网格体!"))
		return;
	}

	const int32 LODNum = StaticMeshComponent->GetStaticMesh()->GetNumLODs();
	StaticMeshComponent->SetLODDataCount(LODNum, LODNum);
	
	if (!StaticMeshComponent->LODData.IsValidIndex(LODIndex))
	{
		// 无效 LOD 索引
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，LOD Index 无效!"))
		return;
	}
	// 获取 LOD 信息
	FStaticMeshComponentLODInfo& LODInfo = StaticMeshComponent->LODData[LODIndex];
	
	// 获取网格体顶点颜色数组
	TArray<FColor> VertexColors = GetStaticMeshVertexColors(StaticMeshComponent, LODIndex);
	
	for (const FVertexOverrideColorInfo VertexOverrideColorInfo : VertexOverrideColorInfos)
	{
		// 判断索引是否有效
		if (VertexColors.IsValidIndex(VertexOverrideColorInfo.VertexIndex))
		{
			VertexColors[VertexOverrideColorInfo.VertexIndex] = VertexOverrideColorInfo.OverrideColor;
		}
	}

	// 必须需进行这一步处理，否则会报错
	LODInfo.OverrideVertexColors = new FColorVertexBuffer;
	LODInfo.OverrideVertexColors->InitFromColorArray(VertexColors);
	// ====
	
	// 通知重新初始化
	// note：如果想要顶点颜色正确显示在网格体模型上，那么应该创建一个材质，并在材质上使用"Vertex Color"节点获取顶点颜色，并连接到"基础颜色"上，"自发光"不能正确显示顶点颜色，必须连接"基础颜色"。
	BeginInitResource(LODInfo.OverrideVertexColors);
	
	// 将渲染状态标记为脏 - 将在帧结束时发送到渲染线程。
	StaticMeshComponent->MarkRenderStateDirty();
	
	// 如果为 true，则不允许在此实例上进行网格绘制。设置是否在构造脚本中覆盖顶点颜色。
	StaticMeshComponent->bDisallowMeshPaintPerInstance = true;
}

void UVertexPainterLibrary::OverrideSkeletalMeshVertexColor(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex,
	TArray<FVertexOverrideColorInfo> VertexOverrideColorInfos)
{
	if (!SkeletalMeshComponent)
	{
		// 无效骨骼网格体组件指针
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，骨骼网格体组件指针无效!"))
		return;
	}
	
	if (!SkeletalMeshComponent->SkeletalMesh)
	{
		// 无效骨骼网格体
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，未设置骨骼网格体!"))
		return;
	}
	
	if (!SkeletalMeshComponent->LODInfo.IsValidIndex(LODIndex))
	{
		// 无效 LOD 索引
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点颜色失败，LOD Index 无效!"))
		return;
	}
	
	FSkelMeshComponentLODInfo& LODInfo = SkeletalMeshComponent->LODInfo[LODIndex];
	
	// 获取网格体顶点颜色数组
	TArray<FColor> VertexColors = GetSkeletalMeshVertexColors(SkeletalMeshComponent, LODIndex);

	// 调试
	VertexColors.Init(FColor::White, VertexColors.Num());
	for (const FVertexOverrideColorInfo VertexOverrideColorInfo : VertexOverrideColorInfos)
	{
		// 判断索引是否有效
		if (VertexColors.IsValidIndex(VertexOverrideColorInfo.VertexIndex))
		{
			VertexColors[VertexOverrideColorInfo.VertexIndex] = VertexOverrideColorInfo.OverrideColor;
		}
	}

	// 必须需进行这一步处理，否则会报错
	LODInfo.OverrideVertexColors = new FColorVertexBuffer;
	LODInfo.OverrideVertexColors->InitFromColorArray(VertexColors);
	// 通知重新初始化
	// note：如果想要顶点颜色正确显示在网格体模型上，那么应该创建一个材质，并在材质上使用"Vertex Color"节点获取顶点颜色，并连接到"基础颜色"上，"自发光"不能正确显示顶点颜色，必须连接"基础颜色"。
	BeginInitResource(LODInfo.OverrideVertexColors);
	
	// 将渲染状态标记为脏 - 将在帧结束时发送到渲染线程。
	SkeletalMeshComponent->MarkRenderStateDirty();
}

TArray<FVertexOverrideColorInfo> UVertexPainterLibrary::GetStaticMeshVertexOverrideColorInfoInSphere(UStaticMeshComponent* StaticMeshComponent, int32 LODIndex,
	FVector SphereWorldPosition, float Radius, FLinearColor OverrideColor)
{
	TArray<FVertexOverrideColorInfo> VertexOverrideColorInfos;
	if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
	{
		if (!StaticMeshComponent)
		{
			UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点覆盖颜色信息失败，静态网格体组件无效!"));
		}
		else if (!StaticMeshComponent->GetStaticMesh())
		{
			UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点覆盖颜色信息失败，未设置静态网格模型!"));
		}
		return VertexOverrideColorInfos;
	}

	// 如果不进行这一系列处理，无法通过判断，LODIndex 始终是无效的。
	const int32 LODNum = StaticMeshComponent->GetStaticMesh()->GetNumLODs();
	StaticMeshComponent->SetLODDataCount(LODNum, LODNum);
	if (!StaticMeshComponent->LODData.IsValidIndex(LODIndex))
	{
		// 无效 LOD 索引
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点覆盖颜色信息失败，LOD Index 无效!"))
		return VertexOverrideColorInfos;
	}
	// ===

	// 获取网格体的场景变换(世界变换)
	const FTransform StaticMeshWorldTransform = StaticMeshComponent->GetComponentTransform();
	// 将球体场景位置转换为静态网格体的本地空间位置 - 在静态网格体本地空间进行比较，而不是将静态网格体的每个顶点位置转换成世界坐标(减少运算)
	const FVector SphereLocationInMeshTransform = UKismetMathLibrary::InverseTransformLocation(StaticMeshWorldTransform, SphereWorldPosition);
	
	// 顶点位置缓存信息
	const FPositionVertexBuffer& VertexPositionBuffer = StaticMeshComponent->GetStaticMesh()->GetRenderData()->LODResources[LODIndex].VertexBuffers.PositionVertexBuffer;
	
	// 获取顶点数量
	const int VertexNum = VertexPositionBuffer.GetNumVertices();
	
	// 遍历每个顶点
	for(int32 VertexIndex = 0; VertexIndex < VertexNum; VertexIndex++)
	{
		// 判断顶点位置是否在被绘制区域
		const FVector VertexPosition = static_cast<UE::Math::TVector4<double>>(VertexPositionBuffer.VertexPosition(VertexIndex));
	
		// 实际的距离
		const float Distance = UKismetMathLibrary::Vector_Distance(VertexPosition, SphereLocationInMeshTransform);

		// NormalizedDistance 是为了方便进行插值而计算的数据
		const float NormalizedDistance = Distance / Radius;
	
		// 当 NormalizedDistance > 1 时，说明顶点已不再球体范围内
		if (NormalizedDistance <= 1)
		{
			// 在绘制范围内，收集在绘制球体范围内的顶点
			FVertexOverrideColorInfo VertexOverrideColorInfo;
	
			VertexOverrideColorInfo.VertexIndex = VertexIndex;
			VertexOverrideColorInfo.OverrideColor = OverrideColor.ToFColor(true);
			VertexOverrideColorInfos.Add(VertexOverrideColorInfo);
		}
	}

	return VertexOverrideColorInfos;
}

TArray<FVertexOverrideColorInfo> UVertexPainterLibrary::GetSkeletalMeshVertexOverrideColorInfoInSphere(
	USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, FVector SphereWorldPosition, float Radius,
	FLinearColor OverrideColor)
{
	// note：骨骼网格体获取顶点位置的方式与静态网格体不同

	TArray<FVertexOverrideColorInfo> VertexOverrideColorInfos;
	if (!SkeletalMeshComponent || !SkeletalMeshComponent->SkeletalMesh)
	{
		if (!SkeletalMeshComponent)
		{
			UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点覆盖颜色信息失败，静态网格体组件无效!"));
		}
		else if (!SkeletalMeshComponent->SkeletalMesh)
		{
			UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点覆盖颜色信息失败，未设置静态网格模型!"));
		}
		return VertexOverrideColorInfos;
	}

	// 骨骼网格体初始化所有 LOD info。
	SkeletalMeshComponent->InitLODInfos();

	if (!SkeletalMeshComponent->LODInfo.IsValidIndex(LODIndex))
	{
		// 无效 LOD 索引
		UE_LOG(VertexPainterBlueprintFunctionLibraryLog, Log, TEXT("获取顶点覆盖颜色信息失败，LOD Index 无效!"))
		return VertexOverrideColorInfos;
	}
	// ===

	// 获取网格体的场景变换(世界变换)
	const FTransform StaticMeshWorldTransform = SkeletalMeshComponent->GetComponentTransform();
	// 将球体场景位置转换为静态网格体的本地空间位置 - 在静态网格体本地空间进行比较，而不是将静态网格体的每个顶点位置转换成世界坐标(减少运算)
	const FVector SphereLocationInMeshTransform = UKismetMathLibrary::InverseTransformLocation(StaticMeshWorldTransform, SphereWorldPosition);
	
	FSkeletalMeshLODRenderData& LODRenderData = SkeletalMeshComponent->SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex];
	FSkinWeightVertexBuffer& SkinWeightVertexBuffer = LODRenderData.SkinWeightVertexBuffer;
	int32 VertexNum = SkeletalMeshComponent->SkeletalMesh->GetResourceForRendering()->LODRenderData[LODIndex].GetNumVertices();

	for(int32 VertexIndex = 0; VertexIndex < VertexNum; VertexIndex++)
	{
		// 通过 GetSkinnedVertexPosition 静态函数来获取骨骼网格体蒙皮后的顶点位置
		FVector VertexPosition = static_cast<UE::Math::TVector4<double>>(USkeletalMeshComponent::GetSkinnedVertexPosition(SkeletalMeshComponent, VertexIndex, LODRenderData, SkinWeightVertexBuffer));

		// 实际的距离
		const float Distance = UKismetMathLibrary::Vector_Distance(VertexPosition, SphereLocationInMeshTransform);

		// NormalizedDistance 是为了方便进行插值而计算的数据
		const float NormalizedDistance = Distance / Radius;
	
		// 当 NormalizedDistance > 1 时，说明顶点已不再球体范围内
		if (NormalizedDistance <= 1)
		{
			// 在绘制范围内，收集在绘制球体范围内的顶点
			FVertexOverrideColorInfo VertexOverrideColorInfo;
	
			VertexOverrideColorInfo.VertexIndex = VertexIndex;
			VertexOverrideColorInfo.OverrideColor = OverrideColor.ToFColor(true);
			VertexOverrideColorInfos.Add(VertexOverrideColorInfo);

			// 调试，最好不要打开，会很卡
			// UKismetSystemLibrary::DrawDebugSphere(SkeletalMeshComponent->GetWorld(), UKismetMathLibrary::TransformLocation(StaticMeshWorldTransform, VertexPosition), 10);
		}
	}

	return VertexOverrideColorInfos;
}


// Fill out your copyright notice in the Description page of Project Settings.
#include "CGoGNMesh.h"

// Sets default values
ACGoGNMesh::ACGoGNMesh()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	mesh->bUseAsyncCooking = true;
}

// Called when the game starts or when spawned
void ACGoGNMesh::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACGoGNMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ACGoGNMesh::LoadMeshFromFile(const FString filename)
{
	// Recupere le chemin vers le fichier : Content/Ressources/filename
	char* path = TCHAR_TO_ANSI(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Ressources"), filename));
	// On importe le fichier dans map2, si l'operation echoue la fonction renvoie false
	if (!io::import_OFF(cmap, path))
		return false;


	// Convertis map2 en tableau de vertexes et triangles
	return GenerateProceduralMesh();
}

void ACGoGNMesh::ComputeMeshArrays(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& UV0, TArray<FVector>& normals, TArray<FProcMeshTangent>& tangents, TArray<FColor>& vertexColors)
{
	vertices.SetNum(0);
	triangles.SetNum(0);

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	foreach_cell(cmap, [&](MyFace f) -> bool 
	{
		std::vector<MyVertex> indexes = incident_vertices(cmap, f);

		for (int i = 0; i < indexes.size(); i++)
		{
			const MyVec3& vec = value<MyVec3>(cmap, position, indexes[i]);
			FVector newVec(vec.x(), vec.y(), vec.z());

			int index = index_of(cmap, indexes[i]);
			if (vertices.Num() <= index)
				vertices.SetNum(index + 1);
			vertices[index_of(cmap, indexes[i])] = newVec;
		}

		if (codegree(cmap, f) == 3)
		{
			triangles.Add(index_of(cmap, indexes[0]));
			triangles.Add(index_of(cmap, indexes[2]));
			triangles.Add(index_of(cmap, indexes[1]));
		}
		else
		{
			std::vector<uint32> table_indices = std::vector<uint32>();
			cgogn::geometry::append_ear_triangulation(cmap, f, position.get(), table_indices, [&]() {});
			for (int i=0; i<table_indices.size()-2; i+=3)
			{
				triangles.Add(table_indices[i]);
				triangles.Add(table_indices[i+2]);
				triangles.Add(table_indices[i+1]);
			}
		}

		return true;
	});

	normals.SetNum(vertices.Num());
	foreach_cell(cmap, [&](MyVertex v) -> bool
	{
		MyVec3 vn = normal(cmap, v, position.get());
		normals[index_of(cmap, v)] = FVector(vn.x(), vn.y(), vn.z());

		return true;
	});
	/*
	// Smooth Shading
	normals.SetNum(vertices.Num());
	for (int i = 0; i < normals.Num(); i++)
		normals[i] = FVector(0.0, 0.0, 0.0);

	for (int i = 0; i < triangles.Num(); i+=3)
	{
		FVector A = vertices[triangles[i]];
		FVector B = vertices[triangles[i+1]];
		FVector C = vertices[triangles[i+2]];
		FVector normal = FVector::CrossProduct(B-A, C-A);
		normal.Normalize();
		normals[triangles[i]] += normal;
		normals[triangles[i+1]] += normal;
		normals[triangles[i+2]] += normal;
	}

	for (int i = 0; i < normals.Num(); i++)
		normals[i].Normalize();

	// Flat Shading
	normals.SetNum(vertices.Num());
	for (int i = 0; i < triangles.Num(); i += 3)
	{
		FVector A = vertices[triangles[i]];
		FVector B = vertices[triangles[i+1]];
		FVector C = vertices[triangles[i+2]];
		FVector normal = FVector::CrossProduct(B-A, C-A);
		normal.Normalize();
		normals[triangles[i]] = normal;
		normals[triangles[i+1]] = normal;
		normals[triangles[i+2]] = normal;
	}
	*/

	UV0.SetNum(0);

	tangents.SetNum(vertices.Num());
	for (int i = 0; i < tangents.Num(); i++)
	{
		FVector normal = normals[i];
		FVector t1 = FVector::CrossProduct(normal, FVector::ForwardVector);
		FVector t2 = FVector::CrossProduct(normal, FVector::UpVector);

		if (t1.Size() > t2.Size())
		{
			t1.Normalize();
			tangents[i] = FProcMeshTangent(t1, true);
		}
		else
		{
			t2.Normalize();
			tangents[i] = FProcMeshTangent(t2, true);
		}
	}

	vertexColors.SetNum(triangles.Num());
	for (int i = 0; i < vertexColors.Num(); i++)
		vertexColors[i] = FColor(1.0, 1.0, 1.0, 1.0);

	// /!\ Quadratic cost
	//UKismetProceduralMeshLibrary::CalculateTangentsForMesh(vertices, triangles, UV0, normals, tangents);
}

bool ACGoGNMesh::GenerateProceduralMesh()
{
	// Recupere l'attribut position des vertexes de map2
	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	//Mesh triangulated_c
	//apply_ear_triangulation(triangulated_cmap, position.get());

	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> UV0;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FColor> vertexColors;

	ComputeMeshArrays(vertices, triangles, UV0, normals, tangents, vertexColors);

	mesh->CreateMeshSection(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);
	mesh->ContainsPhysicsTriMeshData(true);

	return true;
}

bool ACGoGNMesh::UpdateProceduralMesh()
{
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> UV0;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FColor> vertexColors;

	ComputeMeshArrays(vertices, triangles, UV0, normals, tangents, vertexColors);
	mesh->UpdateMeshSection(0, vertices, normals, UV0, vertexColors, tangents);

	return true;
}

TArray<int> ACGoGNMesh::Laplacian(const FVector origin, const float radius)
{
	TArray<int> movedVertices;

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");
	auto laplacian = get_attribute<MyVec3, MyVertex>(cmap, "laplacian");
	if (!laplacian)
		laplacian = add_attribute<MyVec3, MyVertex>(cmap, "laplacian");

	foreach_cell(cmap, [&](MyVertex v) -> bool 
	{
		MyVec3 vertex_pos = value<MyVec3>(cmap, position, v);

		FVector pos(vertex_pos.x(), vertex_pos.y(), vertex_pos.z());
		if (radius < 0 || (origin - pos).Size() < radius)
		{

			std::vector<MyVertex> neighbors = adjacent_vertices_through_edge(cmap, v);

			MyVec3 smoothed = MyVec3(0.0, 0.0, 0.0);

			for (int i = 0; i < neighbors.size(); i++)
			{
				float weight = 1.0;
				MyVec3 neighbor_pos = value<MyVec3>(cmap, position, neighbors[i]);
				smoothed += (1.0 / (float)neighbors.size()) * weight * (neighbor_pos - vertex_pos);
			}

			value<MyVec3>(cmap, laplacian, v) = smoothed;
		}

		return true;
	});

	foreach_cell(cmap, [&](MyVertex v) -> bool
	{
		MyVec3 vertex_pos = value<MyVec3>(cmap, position, v);

		FVector pos(vertex_pos.x(), vertex_pos.y(), vertex_pos.z());
		if (radius <= 0 || (origin - pos).Size() < radius)
		{
			value<MyVec3>(cmap, position, v) += value<MyVec3>(cmap, laplacian, v);
			movedVertices.Add(index_of(cmap, v));
		}
			
		return true;
	});

	UpdateProceduralMesh();

	return movedVertices;
}

TArray<int> ACGoGNMesh::TransfoFace(const int face, const FVector transfo_vector, int transfo_type, const float rayon, const int proportion_type)
{
	// Liste d'indice de tout les vertexes qu'on a traite
	TArray<int> displaced_vertices;

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	// on trouve la bonne face dans cmap
	MyFace f = MyFace(Dart(face));

	// Recuperation des vertex incidents
	std::vector<MyVertex> indexes = incident_vertices(cmap, f);
	// Recuperation des positions d'origine
	MyVec3 vec1pos = value<MyVec3>(cmap, position, indexes[0]);
	MyVec3 vec2pos = value<MyVec3>(cmap, position, indexes[1]);
	MyVec3 vec3pos = value<MyVec3>(cmap, position, indexes[2]);
	// transformation des vertex du triangle
	for (int i = 0; i < indexes.size(); i++)
	{
		if (transfo_type == 0)
		{
			value<MyVec3>(cmap, position, indexes[i]).x() += transfo_vector.X;
			value<MyVec3>(cmap, position, indexes[i]).y() += transfo_vector.Y;
			value<MyVec3>(cmap, position, indexes[i]).z() += transfo_vector.Z;
		}
		else if (transfo_type == 1)
		{
			FVector new_position;
			new_position.X = value<MyVec3>(cmap, position, indexes[i]).x();
			new_position.Y = value<MyVec3>(cmap, position, indexes[i]).y();
			new_position.Z = value<MyVec3>(cmap, position, indexes[i]).z();

			FVector origin;
			// Recuperation du barycentre
			origin.X = ((vec1pos + vec2pos + vec3pos) / 3).x();
			origin.Y = ((vec1pos + vec2pos + vec3pos) / 3).y();
			origin.Z = ((vec1pos + vec2pos + vec3pos) / 3).z();

			new_position = RotateAroundAxis(new_position, origin, transfo_vector.X, 0);
			new_position = RotateAroundAxis(new_position, origin, transfo_vector.Y, 1);
			new_position = RotateAroundAxis(new_position, origin, transfo_vector.Z, 2);

			value<MyVec3>(cmap, position, indexes[i]).x() = new_position.X;
			value<MyVec3>(cmap, position, indexes[i]).y() = new_position.Y;
			value<MyVec3>(cmap, position, indexes[i]).z() = new_position.Z;
		}
		else if (transfo_type == 2)
		{
			value<MyVec3>(cmap, position, indexes[i]).x() *= transfo_vector.X;
			value<MyVec3>(cmap, position, indexes[i]).y() *= transfo_vector.Y;
			value<MyVec3>(cmap, position, indexes[i]).z() *= transfo_vector.Z;
		}

		// Ajoute ces vertex a la liste des vertex traites
		displaced_vertices.Add(index_of(cmap, indexes[i]));
	}

	// Deux piles pour le parcours flow fill
	TArray<MyFace> neighbors_list;
	TArray<MyFace> new_neighbors_list;
	new_neighbors_list.Add(f);

	// Tant qu'on a des elements a traiter
	while (new_neighbors_list.Num() > 0)
	{
		neighbors_list.Add(new_neighbors_list.Pop());

		// Traite les voisins
		while (neighbors_list.Num() > 0)
		{
			MyFace f2 = neighbors_list.Pop();

			// Recuperation des voisins
			foreach_adjacent_face_through_edge(cmap, f2, [&](MyFace f3) -> bool 
			{
				// Recuperation des vertex de la face
				std::vector<MyVertex> indexes = incident_vertices(cmap, f3);
				bool added_neighbor = false;
				for (int i = 0; i < indexes.size(); i++)
				{
					// Position
					MyVec3 pos = value<MyVec3>(cmap, position, indexes[i]);
					// Distance par rapport a l'origine
					float distance = std::min((vec1pos - pos).norm(), std::min((vec2pos - pos).norm(), (vec3pos - pos).norm()));
					// Si le vertex n'as pas encore ete traitee et qu'il est dans le rayon
					if (!displaced_vertices.Contains(index_of(cmap, indexes[i])) && distance < rayon)
					{
						// Calcul du decalage a faire en fonction de proportion_type
						float scale = GetScale(distance, rayon, proportion_type);
						// transformation du vertex
						if (transfo_type == 0)
						{
							value<MyVec3>(cmap, position, indexes[i]).x() += scale * transfo_vector.X;
							value<MyVec3>(cmap, position, indexes[i]).y() += scale * transfo_vector.Y;
							value<MyVec3>(cmap, position, indexes[i]).z() += scale * transfo_vector.Z;
						}
						else if (transfo_type == 1)
						{
							FVector new_position;
							new_position.X = value<MyVec3>(cmap, position, indexes[i]).x();
							new_position.Y = value<MyVec3>(cmap, position, indexes[i]).y();
							new_position.Z = value<MyVec3>(cmap, position, indexes[i]).z();

							FVector origin;
							origin.X = ((vec1pos + vec2pos + vec3pos) / 3).x();
							origin.Y = ((vec1pos + vec2pos + vec3pos) / 3).y();
							origin.Z = ((vec1pos + vec2pos + vec3pos) / 3).z();

							new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.X, 0);
							new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.Y, 1);
							new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.Z, 2);

							value<MyVec3>(cmap, position, indexes[i]).x() = new_position.X;
							value<MyVec3>(cmap, position, indexes[i]).y() = new_position.Y;
							value<MyVec3>(cmap, position, indexes[i]).z() = new_position.Z;
						}
						else if (transfo_type == 2)
						{
							value<MyVec3>(cmap, position, indexes[i]).x() *= 1 + scale * (transfo_vector.X - 1);
							value<MyVec3>(cmap, position, indexes[i]).y() *= 1 + scale * (transfo_vector.Y - 1);
							value<MyVec3>(cmap, position, indexes[i]).z() *= 1 + scale * (transfo_vector.Z - 1);
						}

						// On range le voisin dans displaced_vertices pour ne plus y toucher
						displaced_vertices.Add(index_of(cmap, indexes[i]));
						// Propagation
						if (!added_neighbor)
						{
							new_neighbors_list.Add(f3);
							added_neighbor = true;
						}
					}
				}

				return true;
			});
		}
	}

	UpdateProceduralMesh();

	return displaced_vertices;
}

TArray<int> ACGoGNMesh::TransfoEdge(const int edge, const FVector transfo_vector, int transfo_type, const float rayon, const int proportion_type)
{
	TArray<int> displaced_vertices;
	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	// Recuperation du edge d'origine
	MyEdge e = MyEdge(Dart(edge));

	// Recuperation des vertex incidents
	std::vector<MyVertex> indexes = incident_vertices(cmap, e);

	// Recuperation des positions d'origine
	MyVec3 vec1pos = value<MyVec3>(cmap, position, indexes[0]);
	MyVec3 vec2pos = value<MyVec3>(cmap, position, indexes[1]);

	// Decalage des vertex de l'edge
	for (int i = 0; i < indexes.size(); i++)
	{
		if (transfo_type == 0)
		{
			value<MyVec3>(cmap, position, indexes[i]).x() += transfo_vector.X;
			value<MyVec3>(cmap, position, indexes[i]).y() += transfo_vector.Y;
			value<MyVec3>(cmap, position, indexes[i]).z() += transfo_vector.Z;
		}
		else if (transfo_type == 1)
		{
			FVector new_position;
			new_position.X = value<MyVec3>(cmap, position, indexes[i]).x();
			new_position.Y = value<MyVec3>(cmap, position, indexes[i]).y();
			new_position.Z = value<MyVec3>(cmap, position, indexes[i]).z();

			FVector origin;
			origin.X = ((vec1pos + vec2pos) / 2).x();
			origin.Y = ((vec1pos + vec2pos) / 2).y();
			origin.Z = ((vec1pos + vec2pos) / 2).z();

			new_position = RotateAroundAxis(new_position, origin, transfo_vector.X, 0);
			new_position = RotateAroundAxis(new_position, origin, transfo_vector.Y, 1);
			new_position = RotateAroundAxis(new_position, origin, transfo_vector.Z, 2);

			value<MyVec3>(cmap, position, indexes[i]).x() = new_position.X;
			value<MyVec3>(cmap, position, indexes[i]).y() = new_position.Y;
			value<MyVec3>(cmap, position, indexes[i]).z() = new_position.Z;
		}
		else if (transfo_type == 2)
		{
			value<MyVec3>(cmap, position, indexes[i]).x() *= transfo_vector.X;
			value<MyVec3>(cmap, position, indexes[i]).y() *= transfo_vector.Y;
			value<MyVec3>(cmap, position, indexes[i]).z() *= transfo_vector.Z;
		}

		// Ajoute ces vertex a la liste des vertex traites
		displaced_vertices.Add(index_of(cmap, indexes[i]));
	}
	TArray<int> already_visited;

	// Deux piles pour le parcours flow fill
	TArray<MyEdge> neighbors_list;
	TArray<MyEdge> new_neighbors_list;
	new_neighbors_list.Add(e);

	// Tant qu'on a des elements a traiter
	while (new_neighbors_list.Num() > 0)
	{
		neighbors_list.Add(new_neighbors_list.Pop());

		// Traite les voisins
		while (neighbors_list.Num() > 0)
		{
			MyEdge e2 = neighbors_list.Pop();
			already_visited.Add(e.dart.index);

			// Recuperation des voisins
			foreach_incident_vertex(cmap, e2, [&](MyVertex v) -> bool 
			{
				foreach_incident_edge(cmap, v, [&](MyEdge e3) -> bool 
				{
					if (!already_visited.Contains(e3.dart.index))
					{
						bool added_neighbor = false;
						std::vector<MyVertex> indexes = incident_vertices(cmap, e3);
						for (int i = 0; i < indexes.size(); i++)
						{
							// Position
							MyVec3 pos = value<MyVec3>(cmap, position, indexes[i]);
							// Distance par rapport a l'origine
							float distance = std::min((vec1pos - pos).norm(), (vec2pos - pos).norm());
							// Si le vertex n'as pas encore ete traitee et qu'il est dans le rayon
							if (!displaced_vertices.Contains(index_of(cmap, indexes[i])) && distance < rayon)
							{
								// Recuperation du scale en fonction de proportion_type
								float scale = GetScale(distance, rayon, proportion_type);
								// Decalage du vertex
								if (transfo_type == 0)
								{
									value<MyVec3>(cmap, position, indexes[i]).x() += scale * transfo_vector.X;
									value<MyVec3>(cmap, position, indexes[i]).y() += scale * transfo_vector.Y;
									value<MyVec3>(cmap, position, indexes[i]).z() += scale * transfo_vector.Z;
								}
								else if (transfo_type == 1)
								{
									FVector new_position;
									new_position.X = value<MyVec3>(cmap, position, indexes[i]).x();
									new_position.Y = value<MyVec3>(cmap, position, indexes[i]).y();
									new_position.Z = value<MyVec3>(cmap, position, indexes[i]).z();

									FVector origin;
									origin.X = ((vec1pos + vec2pos) / 2).x();
									origin.Y = ((vec1pos + vec2pos) / 2).y();
									origin.Z = ((vec1pos + vec2pos) / 2).z();

									new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.X, 0);
									new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.Y, 1);
									new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.Z, 2);

									value<MyVec3>(cmap, position, indexes[i]).x() = new_position.X;
									value<MyVec3>(cmap, position, indexes[i]).y() = new_position.Y;
									value<MyVec3>(cmap, position, indexes[i]).z() = new_position.Z;
								}
								else if (transfo_type == 2)
								{
									value<MyVec3>(cmap, position, indexes[i]).x() *= 1 + scale * (transfo_vector.X - 1);
									value<MyVec3>(cmap, position, indexes[i]).y() *= 1 + scale * (transfo_vector.Y - 1);
									value<MyVec3>(cmap, position, indexes[i]).z() *= 1 + scale * (transfo_vector.Z - 1);
								}

								// On range le voisin dans displaced_vertices pour ne plus y toucher
								displaced_vertices.Add(index_of(cmap, indexes[i]));
								// Propagation
								if (!added_neighbor)
								{
									new_neighbors_list.Add(e3);
									added_neighbor = false;
								}
							}
						}
					}
					return true;
				});
				return true;
			});
		}
	}
	UpdateProceduralMesh();

	return displaced_vertices;
}

TArray<int> ACGoGNMesh::TransfoVertex(const int vertex, const FVector transfo_vector, int transfo_type, const float rayon, const int proportion_type)
{
	TArray<int> displaced_vertices;
	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	// Recuperation du vertex d'origine
	foreach_cell(cmap, [&](MyVertex v) -> bool 
	{
		// vertex d'origine trouvee
		if (index_of(cmap, v) == vertex)
		{
			// Recuperation de la position d'origine
			MyVec3 origin_position = value<MyVec3>(cmap, position, v);
			// Decalage du vertex d'origine
			FVector new_position;
			FVector origin;
			switch (transfo_type)
			{
			case 0:
				value<MyVec3>(cmap, position, v).x() += transfo_vector.X;
				value<MyVec3>(cmap, position, v).y() += transfo_vector.Y;
				value<MyVec3>(cmap, position, v).z() += transfo_vector.Z;
				break;
			case 1:
				new_position.X = value<MyVec3>(cmap, position, v).x();
				new_position.Y = value<MyVec3>(cmap, position, v).y();
				new_position.Z = value<MyVec3>(cmap, position, v).z();


				origin.X = origin_position.x();
				origin.Y = origin_position.y();
				origin.Z = origin_position.z();

				new_position = RotateAroundAxis(new_position, origin, transfo_vector.X, 0);
				new_position = RotateAroundAxis(new_position, origin, transfo_vector.Y, 1);
				new_position = RotateAroundAxis(new_position, origin, transfo_vector.Z, 2);

				value<MyVec3>(cmap, position, v).x() = new_position.X;
				value<MyVec3>(cmap, position, v).y() = new_position.Y;
				value<MyVec3>(cmap, position, v).z() = new_position.Z;
				break;
			case 2:
				value<MyVec3>(cmap, position, v).x() *= transfo_vector.X;
				value<MyVec3>(cmap, position, v).y() *= transfo_vector.Y;
				value<MyVec3>(cmap, position, v).z() *= transfo_vector.Z;
				break;
			}

			// Ajout du vertex a la liste des vertex traites
			displaced_vertices.Add(index_of(cmap, v));

			// Deux piles pour le parcours flow fill
			TArray<MyVertex> neighbors_list;
			TArray<MyVertex> new_neighbors_list;
			// Ajout du vertex d'origine a la pile
			new_neighbors_list.Add(v);

			// Tant qu'on a des elements a traiter
			while (new_neighbors_list.Num() > 0)
			{
				neighbors_list.Add(new_neighbors_list.Pop());

				// Traite les voisins
				while (neighbors_list.Num() > 0)
				{
					MyVertex v2 = neighbors_list.Pop();

					// Recuperation des voisins
					foreach_adjacent_vertex_through_edge(cmap, v2, [&](MyVertex v3) -> bool {
						// Position
						MyVec3 vertex_position = value<MyVec3>(cmap, position, v3);
						// Distance par rapport a l'origine
						float distance = (origin_position - vertex_position).norm();
						// Si le voisin n'as pas encore ete traitee et qu'il est dans le rayon
						if (!displaced_vertices.Contains(index_of(cmap, v3)) && distance < rayon)
						{
							// Calcul du decalage a faire en fonction de proportion_type
							float scale = GetScale(distance, rayon, proportion_type);
							switch (transfo_type)
							{
							case 0:
								value<MyVec3>(cmap, position, v3).x() += scale * transfo_vector.X;
								value<MyVec3>(cmap, position, v3).y() += scale * transfo_vector.Y;
								value<MyVec3>(cmap, position, v3).z() += scale * transfo_vector.Z;
								break;
							case 1:
								new_position.X = value<MyVec3>(cmap, position, v3).x();
								new_position.Y = value<MyVec3>(cmap, position, v3).y();
								new_position.Z = value<MyVec3>(cmap, position, v3).z();

								origin.X = origin_position.x();
								origin.Y = origin_position.y();
								origin.Z = origin_position.z();

								new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.X, 0);
								new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.Y, 1);
								new_position = RotateAroundAxis(new_position, origin, scale * transfo_vector.Z, 2);

								value<MyVec3>(cmap, position, v3).x() = new_position.X;
								value<MyVec3>(cmap, position, v3).y() = new_position.Y;
								value<MyVec3>(cmap, position, v3).z() = new_position.Z;
								break;
							case 2:
								value<MyVec3>(cmap, position, v3).x() *= 1 + scale * (transfo_vector.X - 1);
								value<MyVec3>(cmap, position, v3).y() *= 1 + scale * (transfo_vector.Y - 1);
								value<MyVec3>(cmap, position, v3).z() *= 1 + scale * (transfo_vector.Z - 1);
								break;
							}
							// On range le voisin dans displaced_vertices pour ne plus y toucher
							displaced_vertices.Add(index_of(cmap, v3));
							// Propagation
							new_neighbors_list.Add(v3);
						}
						return true;
					});
				}
			}
		}

		return true;
	});

	UpdateProceduralMesh();

	return displaced_vertices;
}

void ACGoGNMesh::Subdivide(const FVector origin, const float radius)
{
	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	// Comme on vas dedoubler chaque Edges, pour eviter d'appliquer l'operations
	// sur les nouveaux edges a l'infini, on vas utiliser un cache
	// Recupere les Edges de cmap dans un cache
	CellCache<Mesh> cache(cmap);
	cache.template build<MyEdge>();

	// On boucle sur tout les Edges du cache
	foreach_cell(cache, [&](MyEdge e) -> bool {
		std::vector<MyVertex> incidents = incident_vertices(cmap, e);
		MyVec3 point1 = value<MyVec3>(cmap, position, incidents[0]);
		MyVec3 point2 = value<MyVec3>(cmap, position, incidents[1]);

		FVector vec1(point1.x(), point1.y(), point1.z());
		FVector vec2(point2.x(), point2.y(), point2.z());

		float distance = std::max((vec1 - origin).Size(), (vec2 - origin).Size());

		if (radius <= 0 || distance < radius)
		{
			// Recuperation des vertexes composant l'edge e
			std::vector<MyVertex> indexes = incident_vertices(cmap, e);
			// Coupe l'edge en 2, stocke le vertex cree dans v
			MyVertex v = cut_edge(cmap, e);

			// Positionne v au centre des 2 vertexes composant l'edge
			value<MyVec3>(cmap, position, v) = (value<MyVec3>(cmap, position, indexes[0]) + value<MyVec3>(cmap, position, indexes[1])) / 2;
		}

		return true;
		});

	GenerateProceduralMesh();
}

void ACGoGNMesh::Simplify(const int nb_vertices, const float max_length)
{
	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	// On vas trier tout les Edges de map2
	// Pour cela on vas creer un EdgeQueue
	CellQueue<MyEdge> edge_queue;
	using EdgeQueueInfo = typename CellQueue<MyEdge>::CellQueueInfo;
	auto edge_queue_info = add_attribute<EdgeQueueInfo, MyEdge>(cmap, "__decimate_edge_queue_info");
	// Recupere la longueur d'un edge
	auto edge_cost = [&](MyEdge e) -> float { return geometry::length(cmap, e, position.get()); };

	// On trie tout les Edges par rapport a leur longueur
	foreach_cell(cmap, [&](MyEdge e) -> bool {

		update_edge_queue(cmap, e, edge_queue, edge_queue_info.get(), edge_cost);

		return true;
		});

	// Compteur du nombre de vertexes supprimes
	int count = 0;
	for (auto it = edge_queue.begin(); it != edge_queue.end(); ++it)
	{
		float length = geometry::length(cmap, *it, position.get());

		// On quitte si on a atteind le nombre de vertices a supprimer
		// comme edge_queue est trie, si on tombe sur un Edge de longueur > max_length
		// on peut aussi quitter
		if ((nb_vertices > 0 && count >= nb_vertices) || (max_length > 0 && length > max_length))
			break;

		std::vector<MyVertex> indexes = incident_vertices(cmap, *it);
		// On positionnera le nouveau vertex au centre des deux anciens vertexes
		MyVec3 new_pos = (value<MyVec3>(cmap, position, indexes[0]) + value<MyVec3>(cmap, position, indexes[1])) / 2;

		MyEdge e1, e2;
		pre_collapse_edge_length(cmap, *it, e1, e2, edge_queue, edge_queue_info.get());
		MyVertex v = collapse_edge(cmap, *it);
		value<Vec3>(cmap, position.get(), v) = new_pos;
		post_collapse_edge_length(cmap, e1, e2, edge_queue, edge_queue_info.get(), edge_cost);


		//remove_dart(cmap, indexes[0].dart);
		//remove_dart(cmap, indexes[1].dart);

		count++;
	}

	remove_attribute<MyEdge>(cmap, edge_queue_info);

	//index_cells<MyVertex, CMap2>(cmap);
	//set_index(CMapBase & m, Dart d, uint32 index)

	foreach_cell(cmap, [&](MyEdge e) -> bool {
		update_edge_queue(cmap, e, edge_queue, edge_queue_info.get(), edge_cost);
		return true;
		});

	GenerateProceduralMesh();
}

int ACGoGNMesh::ExtrudeFace(const int face)
{
	// Recuperation de la face a extrude
	MyFace f = MyFace(Dart(face));
	// vertexes de la face d'origine
	std::vector<MyVertex> indexes = incident_vertices(cmap, f);

	// 1 face qui vas remplacer la face a extrude
	MyFace extruded_face = add_face(cmap, indexes.size(), true);
	// vertexes de la nouvelle face
	std::vector<MyVertex> indexes_new = incident_vertices(cmap, extruded_face);

	// n faces quadrilatere cree par l'extrusion
	TArray<MyFace> new_faces = TArray<MyFace>();
	for (int i = 0; i < indexes.size(); i++)
		new_faces.Add(add_face(cmap, 4, true));

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	// On positionne la nouvelle face correctement
	for (int i = 0; i < indexes.size(); i++)
	{
		// Position
		MyVec3 pos = value<MyVec3>(cmap, position, indexes[i]);

		value<MyVec3>(cmap, position, indexes_new[i]).x() = pos.x();
		value<MyVec3>(cmap, position, indexes_new[i]).y() = pos.y();
		value<MyVec3>(cmap, position, indexes_new[i]).z() = pos.z();
	}

	// On relie les points entre l'ancienne et la nouvelle face
	for (int i = 0; i < indexes.size(); i++)
	{
		std::vector<MyVertex> new_vertices = incident_vertices(cmap, new_faces[i]);
		value<MyVec3>(cmap, position, new_vertices[0]) = value<MyVec3>(cmap, position, indexes[i]);
		value<MyVec3>(cmap, position, new_vertices[1]) = value<MyVec3>(cmap, position, indexes[(i+1)%indexes.size()]);
		value<MyVec3>(cmap, position, new_vertices[2]) = value<MyVec3>(cmap, position, indexes_new[(i+1)%indexes.size()]);
		value<MyVec3>(cmap, position, new_vertices[3]) = value<MyVec3>(cmap, position, indexes_new[i]);

		set_index(cmap, new_vertices[0], index_of(cmap, indexes[i]));
		set_index(cmap, new_vertices[1], index_of(cmap, indexes[(i+1)%indexes.size()]));
		set_index(cmap, new_vertices[2], index_of(cmap, indexes_new[(i+1)%indexes.size()]));
		set_index(cmap, new_vertices[3], index_of(cmap, indexes_new[i]));

		new_vertices[0].dart = indexes[i].dart;
		new_vertices[1].dart = indexes[(i+1)%indexes.size()].dart;
		new_vertices[2].dart = indexes_new[(i+1)%indexes.size()].dart;
		new_vertices[3].dart = indexes_new[i].dart;
	}

	// Suppression de l'ancienne face
	remove_face(cmap, f);

	GenerateProceduralMesh();

	return extruded_face.dart.index;
}

void ACGoGNMesh::CutEdge(const FVector origin, const float radius, const float max_length)
{
	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	bool found_smaller = true;
	while (found_smaller)
	{
		found_smaller = false;

		foreach_cell(cmap, [&](MyEdge e) -> bool {
			std::vector<MyVertex> incidents = incident_vertices(cmap, e);
			MyVec3 point1 = value<MyVec3>(cmap, position, incidents[0]);
			MyVec3 point2 = value<MyVec3>(cmap, position, incidents[1]);

			float length = (point1 - point2).norm();

			FVector vec1(point1.x(), point1.y(), point1.z());
			FVector vec2(point2.x(), point2.y(), point2.z());

			float distance = std::min((vec1 - origin).Size(), (vec2 - origin).Size());

			if (length > max_length && (radius <= 0 || distance < radius))
			{
				found_smaller = true;

				MyVertex v = cut_edge(cmap, e);

				// Positionne v au centre des 2 vertexes composant l'edge
				value<MyVec3>(cmap, position, v) = (point1 + point2) / 2;
			}

			return true;
		});
	}

	GenerateProceduralMesh();
}

int ACGoGNMesh::ClosestVertice(const FVector origin, const float radius)
{
	MyVec3 myOrigin;
	myOrigin.x() = origin.X;
	myOrigin.y() = origin.Y;
	myOrigin.z() = origin.Z;

	int result = -1;
	float minDist = -1;

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	foreach_cell(cmap, [&](MyVertex v) -> bool
	{
		MyVec3 pos = value<MyVec3>(cmap, position, v);
		float dist = (myOrigin - pos).norm();
		if (dist<=radius && (minDist<0 || dist<minDist))
		{
			minDist = dist;
			result = index_of(cmap, v);
		}

		return true;
	});

	return result;
}

int ACGoGNMesh::ClosestEdge(const FVector origin, const float radius)
{
	MyVec3 myOrigin;
	myOrigin.x() = origin.X;
	myOrigin.y() = origin.Y;
	myOrigin.z() = origin.Z;

	int result = -1;
	float minDist = -1;

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	foreach_cell(cmap, [&](MyEdge e) -> bool
	{
		std::vector<MyVertex> indexes = incident_vertices(cmap, e);
		MyVec3 pos1 = value<MyVec3>(cmap, position, indexes[0]);
		MyVec3 pos2 = value<MyVec3>(cmap, position, indexes[1]);
		MyVec3 pos = (pos1+pos2)/2;
		float dist = (myOrigin - pos).norm();
		if (dist<=radius && (minDist<0 || dist<minDist))
		{
			minDist = dist;
			//result = index_of(cmap, e);
			result = e.dart.index;
		}

		return true;
	});

	return result;
}

int ACGoGNMesh::ClosestFace(const FVector origin, const float radius)
{
	MyVec3 myOrigin;
	myOrigin.x() = origin.X;
	myOrigin.y() = origin.Y;
	myOrigin.z() = origin.Z;

	int result = -1;
	float minDist = -1;

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	foreach_cell(cmap, [&](MyFace f) -> bool
	{
		std::vector<MyVertex> indexes = incident_vertices(cmap, f);

		MyVec3 pos(0,0,0);

		for (int i = 0; i < indexes.size(); i++)
		{
			pos += value<MyVec3>(cmap, position, indexes[i]);
		}
		pos /= indexes.size();
		float dist = (myOrigin - pos).norm();
		if (dist<=radius && (minDist<0 || dist<minDist))
		{
			minDist = dist;
			//result = index_of(cmap, f);
			result = f.dart.index;
		}

		return true;
	});

	return result;
}

TArray<FVector> ACGoGNMesh::GetVertices()
{
	TArray<FVector> result = TArray<FVector>();

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	foreach_cell(cmap, [&](MyVertex v) -> bool
	{
		MyVec3 pos = value<MyVec3>(cmap, position, v);
		FVector unrealPos = FVector(pos.x(), pos.y(), pos.z());
		if((int)index_of(cmap, v) >= result.Num())
			result.SetNum(index_of(cmap, v)+1);
		result[index_of(cmap, v)] = unrealPos;

		return true;
	});

	return result;
}

TArray<FVector> ACGoGNMesh::GetEdges()
{
	TArray<FVector> result = TArray<FVector>();

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	foreach_cell(cmap, [&](MyEdge e) -> bool
	{
		std::vector<MyVertex> indexes = incident_vertices(cmap, e);
		MyVec3 pos1 = value<MyVec3>(cmap, position, indexes[0]);
		MyVec3 pos2 = value<MyVec3>(cmap, position, indexes[1]);
		
		FVector unrealPos1 = FVector(pos1.x(), pos1.y(), pos1.z());
		FVector unrealPos2 = FVector(pos2.x(), pos2.y(), pos2.z());

		result.Add(unrealPos1);
		result.Add(unrealPos2);

		return true;
	});

	return result;
}

TArray<FVector> ACGoGNMesh::GetFacesNormals()
{
	TArray<FVector> result = TArray<FVector>();

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	MyVec3 pos(0, 0, 0);

	foreach_cell(cmap, [&](MyFace f) -> bool
	{
		std::vector<MyVertex> indexes = incident_vertices(cmap, f);
		
		// Calcul du Centroid TODO ne fonctionne que sur des polygones concaves
		TArray<FVector> unrealVertices;
		for (int i = 0; i < indexes.size(); i++)
		{
			MyVec3 vec = value<MyVec3>(cmap, position, indexes[i]);
			unrealVertices.Add(FVector(vec.x(), vec.y(), vec.z()));
		}
		FVector unrealPos = PolygonCentroid(unrealVertices);
		result.Add(unrealPos);

		// Calcul de la normale
		MyVec3 vec1 = value<MyVec3>(cmap, position, indexes[1]) - value<MyVec3>(cmap, position, indexes[0]); 
		FVector uevec1 = FVector(vec1.x(), vec1.y(), vec1.z());

		MyVec3 vec2 = value<MyVec3>(cmap, position, indexes[2]) - value<MyVec3>(cmap, position, indexes[0]);
		FVector uevec2 = FVector(vec2.x(), vec2.y(), vec2.z());
		FVector normal = FVector::CrossProduct(uevec1, uevec2);
		normal.Normalize();
		normal *= 5.0;

		result.Add(unrealPos + normal);

		return true;
	});

	return result;
}

void ACGoGNMesh::GetCellNums(int& vertices, int& edges, int& faces)
{
	vertices = 0;
	edges = 0;
	faces = 0;

	foreach_cell(cmap, [&](MyVertex v) -> bool
	{
			vertices++;

		return true;
	});

	foreach_cell(cmap, [&](MyEdge e) -> bool
	{
		edges++;

		return true;
	});

	foreach_cell(cmap, [&](MyFace f) -> bool
	{
		faces++;

		return true;
	});
}

int ACGoGNMesh::GetBorderNum()
{
	int ret = 0;

	TArray<int> edges_bords;
	TArray<int> marked_edges;

	// Recupere toutes les arettes sur les bords
	foreach_cell(cmap, [&](MyEdge e) -> bool
	{
		std::vector<MyFace> faces = incident_faces(cmap, e);
		if (faces.size() <= 1)
			edges_bords.Add(phi2(cmap, e.dart).index);

		return true;
	});

	for(int i=0; i<edges_bords.Num(); i++)
	{
		if(marked_edges.Contains(edges_bords[i]))
			continue;

		ret++;
		marked_edges.Add(edges_bords[i]);
		MyEdge start = MyEdge(Dart(edges_bords[i]));
		MyEdge e = MyEdge(phi1(cmap, start.dart));
		while(e.dart.index != start.dart.index)
		{
			marked_edges.Add(e.dart.index);
			e = MyEdge(phi1(cmap, e.dart));
		}
	}

	return ret;
}

int ACGoGNMesh::GetCCNum()
{
	int ret = 0;

	TArray<int> checked_vertices;

	foreach_cell(cmap, [&](MyVertex v) -> bool
	{
		// Si vertex deja check
		if(checked_vertices.Contains(index_of(cmap, v)))
			return true;

		// Sinon debut d'une nouvelle CC
		checked_vertices.Add(index_of(cmap, v));
		ret++;

		// Deux piles pour le parcours flow fill
		TArray<MyVertex> neighbors_list;
		TArray<MyVertex> new_neighbors_list;
		// Ajout du vertex d'origine a la pile
		new_neighbors_list.Add(v);

		// Tant qu'on a des elements a traiter
		while (new_neighbors_list.Num() > 0)
		{
			neighbors_list.Add(new_neighbors_list.Pop());

			// Traite les voisins
			while (neighbors_list.Num() > 0)
			{
				MyVertex v2 = neighbors_list.Pop();

				// Recuperation des voisins
				foreach_adjacent_vertex_through_edge(cmap, v2, [&](MyVertex v3) -> bool 
				{
					if(!checked_vertices.Contains(index_of(cmap, v3)))
					{
						// On range le voisin dans displaced_vertices pour ne plus y toucher
						checked_vertices.Add(index_of(cmap, v3));
						// Propagation
						new_neighbors_list.Add(v3);
						
					}
					return true;
				});
			}
		}
		return true;
	});

	return ret;
}

TArray<FVector> ACGoGNMesh::DartsPositions(float position_scale, float length_scale)
{
	TArray<FVector> result = TArray<FVector>();

	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	foreach_cell(cmap, [&](MyFace f) -> bool
	{
		std::vector<MyVertex> indexes = incident_vertices(cmap, f);

		TArray<FVector> unrealVertices;
		for (int i = 0; i < indexes.size(); i++)
		{
			MyVec3 vec = value<MyVec3>(cmap, position, indexes[i]);
			unrealVertices.Add(FVector(vec.x(), vec.y(), vec.z()));
		}
		FVector center = PolygonCentroid(unrealVertices);

		for (int i = 0; i < indexes.size(); i++)
		{
			// Position des vertex
			FVector V1 = unrealVertices[i];
			FVector V2 = unrealVertices[(i+1)%indexes.size()];
			// Position des vertex du brin suivant pour phi1, le 1er vertex etant V2
			FVector V4 = unrealVertices[(i+2)%indexes.size()];

			// Position des darts
			FVector V1p = (V1-center)*position_scale+center;
			FVector V2p = (V2-center)*position_scale+center;
			FVector V4p = (V4-center)*position_scale+center;

			// Droite du dart de V1 a V2
			result.Add(V1p);
			result.Add(V1p*(1-length_scale)+V2p*length_scale);

			// Droite pour la moitie phi2 (l'autre moitie est dessine par le dart concernee)
			result.Add(V1*0.5+V2*0.5);
			result.Add(V1p*0.5+V2p*0.5);

			// Droite pour le phi1
			result.Add(V1p*(1-length_scale*0.8)+V2p*length_scale*0.8);
			result.Add(V2p*0.8*length_scale+V4p*(1-0.8*length_scale));
		}

		return true;
	});

	return result;
}

void ACGoGNMesh::GetBoundingBox(FVector& start, FVector& end)
{
	auto position = get_attribute<MyVec3, MyVertex>(cmap, "position");

	bool first_loop = true;

	foreach_cell(cmap, [&](MyVertex v) -> bool
	{
		FVector new_position;
		new_position.X = value<MyVec3>(cmap, position, v).x();
		new_position.Y = value<MyVec3>(cmap, position, v).y();
		new_position.Z = value<MyVec3>(cmap, position, v).z();

		if(first_loop)
		{
			start = new_position;
			end = new_position;
			first_loop = false;
		}
		else
		{
			if(new_position.X < start.X)
				start.X = new_position.X;
			if (new_position.Y < start.Y)
				start.Y = new_position.Y;
			if (new_position.Z < start.Z)
				start.Z = new_position.Z;

			if (new_position.X > end.X)
				end.X = new_position.X;
			if (new_position.Y > end.Y)
				end.Y = new_position.Y;
			if (new_position.X > end.X)
				end.Z = new_position.Z;
		}

		return true;
	});
}

float ACGoGNMesh::GetScale(const float distance, const float rayon, const int proportion_type)
{
	float distance_rayon = 1 - distance / rayon;
	switch (proportion_type)
	{
		// Constant
	case 0:
		return 1;
		// Random
	case 1:
		return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		// Lineaire
	case 2:
		return distance_rayon;
		// Sharp
	case 3:
		return pow(distance_rayon, 2);
		// Racine
	case 4:
		return sqrt(distance_rayon);
		// Sphere
	case 5:
		return sqrt(2 * distance_rayon - pow(distance_rayon, 2));
		// Smooth
	case 6:
		return 3 * (pow(distance_rayon, 2)) - 2 * (pow(distance_rayon, 3));
	default:
		return 0;
	}
}

FVector ACGoGNMesh::RotateAroundAxis(const FVector vertex, const FVector origin, const float angle, const int axis)
{
	FVector result;
	float rotation;
	float posX;
	float posY;
	float posZ;

	switch (axis)
	{
		// Rotation autour de X
	case 0:
		rotation = angle * 3.141592 / 180;
		posY = vertex.Y - origin.Y;
		posZ = vertex.Z - origin.Z;

		result.X = vertex.X;
		result.Y = posY * cos(rotation) + posZ * sin(rotation) + origin.Y;
		result.Z = -posY * sin(rotation) + posZ * cos(rotation) + origin.Z;
		break;

		// Rotation autour de Y
	case 1:
		rotation = angle * 3.141592 / 180;
		posX = vertex.X - origin.X;
		posZ = vertex.Z - origin.Z;

		result.X = posX * cos(rotation) + posZ * sin(rotation) + origin.X;
		result.Y = vertex.Y;
		result.Z = -posX * sin(rotation) + posZ * cos(rotation) + origin.Z;
		break;

		// Rotation autour de Z
	case 2:
		rotation = angle * 3.141592 / 180;
		posX = vertex.X - origin.X;
		posY = vertex.Y - origin.Y;

		result.X = posX * cos(rotation) + posY * sin(rotation) + origin.X;
		result.Y = -posX * sin(rotation) + posY * cos(rotation) + origin.Y;
		result.Z = vertex.Z;
		break;
	}
	return result;
}

FVector ACGoGNMesh::PolygonCentroid(TArray<FVector> P)
{
	float A2;
	float Areasum2 = 0;
	FVector Cent3;

	FVector res = FVector(0, 0, 0);

	for (int i = 1; i < P.Num() - 1; i++)
	{
		// Centroid x3 du triangle courant
		Cent3.X = P[0].X + P[i].X + P[i + 1].X;
		Cent3.Y = P[0].Y + P[i].Y + P[i + 1].Y;
		Cent3.Z = P[0].Z + P[i].Z + P[i + 1].Z;

		// Aire x2
		FVector AB = P[i] - P[0];
		FVector AC = P[i + 1] - P[0];
		A2 = FVector::DotProduct(AB, AC);
		//A2 = (P[i].X - P[0].X) * (P[i+1].Y - P[0].Y) - (P[i+1].X - P[0].X) * (P[i].Y - P[0].Y);

		res.X += A2 * Cent3[0];
		res.Y += A2 * Cent3[1];
		res.Z += A2 * Cent3[2];
		Areasum2 += A2;
	}

	res.X /= 3 * Areasum2;
	res.Y /= 3 * Areasum2;
	res.Z /= 3 * Areasum2;

	return res;
}